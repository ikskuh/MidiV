#include "midiv.hpp"
#include "hal.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include "midi.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <array>

using hrc = std::chrono::high_resolution_clock;

static std::mutex mididataMutex;
// current state, synchronized state, summed state
static MMidiState mididata;
static MMidiState syncmidi, summidi;

static std::array<MVisualization, 128> visualizations;
static std::chrono::high_resolution_clock::time_point startPoint, lastFrame;

static std::map<std::string, std::function<void(MShader const &, MUniform const &, unsigned int &)>> uniformMappings;

static int width, height;

static float totalTime, deltaTime;

static GLuint backgroundTexture, stageTexture, stageDataTexture;
static GLuint stageTextureSlot, stageDataSlot;

static struct
{
	GLuint texFFT, texChannels;
    GLuint vao, fb;
} resources;

static void nop() { }

static std::map<std::string, MCCTarget> globalCCs;

static uint8_t switchCC = 0xFF;

static void midiCallback( Midi::Message const & msg);

static struct
{
	int from, to;
	double progress;
	double speed;
} blending;

void APIENTRY msglog(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    (void)source;
    (void)type;
    (void)id;
    (void)severity;
    (void)userParam;

    static std::string previousMessage;

    std::string currentMessage(message, length);

    if(currentMessage != previousMessage)
    {
        fprintf(stderr, "[OpenGL] ");
        fwrite(message, length, 1, stderr);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
    previousMessage = currentMessage;

    if(severity == GL_DEBUG_SEVERITY_HIGH)
        nop(); // point to break on
}

void MidiV::Initialize(nlohmann::json const & config)
{
	// Load the global CC overrides
	for(auto const & override : config["bindings"])
	{
		auto from = override["uniform"].get<std::string>();

		auto target = MCCTarget::load(override);

		globalCCs.emplace(from, target);
	}

	switchCC = uint8_t(Utils::get(config, "vis-switch-cc", 0xFF));
	blending.speed = Utils::get(config, "vis-switch-time", 0.5);

    glDebugMessageCallback(msglog, nullptr);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glCreateVertexArrays(1, &resources.vao);
    glBindVertexArray(resources.vao);

    {
        glCreateTextures(GL_TEXTURE_1D, 1, &resources.texFFT);
        glTextureStorage1D(
            resources.texFFT,
            7,
            GL_RG32F,
            128);
        glTextureParameteri(resources.texFFT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(resources.texFFT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(resources.texFFT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTextureParameteri(resources.texFFT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(resources.texFFT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }

    {
        glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &resources.texChannels);
        glTextureStorage2D(
            resources.texChannels,
            7,
            GL_RGBA32F,
            128, 16);
        glTextureParameteri(resources.texChannels, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTextureParameteri(resources.texChannels, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(resources.texChannels, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTextureParameteri(resources.texChannels, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(resources.texChannels, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }



    glCreateFramebuffers(1, &resources.fb);

    startPoint = hrc::now();
    lastFrame = startPoint;

    uniformMappings["uTime"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_FLOAT);
        glProgramUniform1f(sh.program, u.position, totalTime);
	};
    uniformMappings["uDeltaTime"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_FLOAT);
        glProgramUniform1f(sh.program, u.position, deltaTime);
	};
	uniformMappings["uPitchWheel"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_FLOAT);
        glProgramUniform1f(sh.program, u.position, float(syncmidi.pitch));
	};
    uniformMappings["uScreenSize"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_INT_VEC2);
        glProgramUniform2i(sh.program, u.position, width, height);
	};
    uniformMappings["uBackground"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_2D);
		glProgramUniform1i(sh.program, u.position, slot);
        glBindTextureUnit(slot, backgroundTexture);
		slot++;
	};
	uniformMappings["uStage"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_2D);
		glProgramUniform1i(sh.program, u.position, slot);
        glBindTextureUnit(slot, stageTexture);
		stageTextureSlot = slot;
		slot++;
	};
	uniformMappings["uStageData"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_2D);
		glProgramUniform1i(sh.program, u.position, slot);
        glBindTextureUnit(slot, stageDataTexture);
		stageDataSlot = slot;
		slot++;
	};
    uniformMappings["uFFT"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_1D);
		glProgramUniform1i(sh.program, u.position, slot);
		glBindTextureUnit(slot, resources.texFFT);
		slot++;
	};
    uniformMappings["uChannels"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_1D_ARRAY);
		glProgramUniform1i(sh.program, u.position, slot);
		glBindTextureUnit(slot, resources.texChannels);
		slot++;
	};


	Midi::RegisterCallback(0x00, midiCallback);
}


void MidiV::LoadVisualization(int slot, std::string const & fileName)
{
    auto current = HAL::GetWorkingDirectory();
    auto file = Utils::LoadFile<char>(fileName);

    if(file)
    {
		auto dir = HAL::GetDirectoryOf(fileName);

		HAL::SetWorkingDirectory(dir);

        auto data = nlohmann::json::parse(file->begin(), file->end());

        visualizations[slot] = MVisualization(data);
	}
	else
	{
        Log() << fileName << "not found!";
	}

    HAL::SetWorkingDirectory(current);
}

void MidiV::Resize(int w, int h)
{
    Log() << "Resize to " << w << "*" << h;
	glViewport(0, 0, w, h);
    for(auto & vis : visualizations)
	{
		vis.resize(w, h);
	}
    width = w;
    height = h;
}

void MCCTarget::update(double deltaTime)
{
	switch(this->type)
	{
		case MCCTarget::Unknown:
			this->value = 0;
			break;
		case MCCTarget::Fixed:
			this->value = this->fixed;
			break;
		case MCCTarget::CC:
			if(this->hasChannel())
				this->value = syncmidi.channels[this->channel].ccs[this->index];
			else
				this->value = syncmidi.ccs[this->index];
			break;
		case MCCTarget::Note:
			this->value = syncmidi.channels[this->channel].notes[this->index].value;
			break;
		case MCCTarget::Pitch:
			if(this->hasChannel())
				this->value= syncmidi.channels[this->channel].pitch;
			else
				this->value = syncmidi.pitch;
			break;
	}

	this->value *= this->mapping.high;
	this->value += this->mapping.low;

	this->sum_value += deltaTime * this->value;
}

static void bindCCUniform(GLuint pgm, MUniform const & uniform, MCCTarget const & cc)
{
	switch(uniform.type)
	{
		case GL_FLOAT:
			glProgramUniform1f(pgm, uniform.position, GLfloat(cc.get()));
			break;
		case GL_DOUBLE:
			glProgramUniform1d(pgm, uniform.position, GLdouble(cc.get()));
			break;

		case GL_INT:
			glProgramUniform1i(pgm, uniform.position, GLint(127.0 * cc.get()));
			break;
		case GL_UNSIGNED_INT:
			glProgramUniform1ui(pgm, uniform.position, GLuint(127.0 * cc.get()));
			break;

		default:
			Log() << "Mapped uniform " << uniform.name << " has an unsupported type!";
	}
}

template<typename T>
static void transfer(std::vector<MCCTarget> & dest, T const & src, std::string const & elem)
{
	auto it = src.find(elem);
	if(it != src.end())
		dest.push_back(it->second);
}

static void dump_texture(char const * name, GLuint tex)
{
	unsigned int w = 0, h = 0;
	glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_WIDTH, reinterpret_cast<int*>(&w));
	glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_HEIGHT, reinterpret_cast<int*>(&h));

	std::vector<uint8_t> buffer(3 * w * h);

	glGetTextureImage(
		tex,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		GLsizei(buffer.size()),
		buffer.data());

	std::vector<uint8_t> scanline(3 * w);
	for(unsigned int i = 0; i < (h / 2); i++)
	{
		unsigned int y0 = i;
		unsigned int y1 = h - 1 - i;

		memcpy(scanline.data(), &buffer[3 * y0 * w], 3 * w);
		memcpy(&buffer[3 * y0 * w], &buffer[3 * y1 * w], 3 * w);
		memcpy(&buffer[3 * y1 * w], scanline.data(), 3 * w);
	}

	FILE * f = fopen(name, "wb");
	fprintf(f, "P6 %d %d 255\n", w, h);
	fwrite(buffer.data(), buffer.size(), 1, f);
	fclose(f);
}

volatile static bool takeScreenshot = false;

void MidiV::TakeScreenshot()
{
	takeScreenshot = true;
}

void MidiV::Render()
{
	auto now = hrc::now();
    totalTime      = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - startPoint).count();
    deltaTime = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame).count();

	// Clone to prevent changes
	{
        std::lock_guard<std::mutex> lock(mididataMutex);

		for(int y = 0; y < 16; y++)
		{
			for(int x = 0; x < 128; x++)
                mididata.channels[y].notes[x].tick(double(deltaTime));
		}

        syncmidi = mididata;
	}

	// Update bindings
	for(auto & cc : globalCCs) cc.second.update(deltaTime);
	for(auto & vis : visualizations)
	{
		for(auto & cc : vis.ccMapping) cc.second.update(deltaTime);
		for(auto & stage : vis.stages)
		{
			for(auto & cc : stage.shader.bindings) cc.second.update(deltaTime);
		}
	}

	// Update resources
	for(auto & vis : visualizations)
	{
		for(auto & res : vis.resources) res.second.update();
		for(auto & stage : vis.stages)
		{
			for(auto & res : stage.shader.resources) res.second.update();
		}
	}

	// Integrate midi data
	for(int c = 0; c < 16; c++)
	{
		for(int n = 0; n < 128; n++)
		{
            summidi.channels[c].ccs[n]         += double(deltaTime) * syncmidi.channels[c].ccs[n];
            summidi.channels[c].notes[n].value += double(deltaTime) * syncmidi.channels[c].notes[n].value;
		}
	}

	{
		std::array<glm::vec2, 128> fft;
		glm::vec4 channels[16][128];

		for(unsigned int x = 0; x < 128; x++)
		{
			fft[x] = glm::vec2(0);
			for(unsigned int y = 0; y < 16; y++)
			{
				if(y != 9)
				{
                    fft[x].x += float(syncmidi.channels[y].notes[x].value);
                    fft[x].y += float(summidi.channels[y].notes[x].value);
				}
				channels[y][x].x = float(syncmidi.channels[y].notes[x].value);
				channels[y][x].y = float(syncmidi.channels[y].ccs[x]);
				channels[y][x].z = float(summidi.channels[y].notes[x].value);
				channels[y][x].w = float(summidi.channels[y].ccs[x]);
			}
		}

		glTextureSubImage1D(
			resources.texFFT,
			0,
			0, 128,
			GL_RG,
			GL_FLOAT,
			fft.data());

		static constexpr bool customMipMaps = true;
		if constexpr(customMipMaps)
		{
			auto const sample = [&](double f) {
				int x = int(127.0 * f + 0.5);
				if(x < 0) return glm::vec2(0);
				if(x >= 128) return glm::vec2(0);
				return fft[size_t(x)];
			};

			auto const gauss = [&](double f) {
				auto constexpr d = (0.5 / 127.0);
				return 0.06136f * sample(f - 2 * d)
					+  0.24477f * sample(f - 1 * d)
					+  0.38774f * sample(f)
					+  0.24477f * sample(f + 1 * d)
					+  0.06136f * sample(f + 2 * d)
					;
			};

			// custom mipmap algorithm:
			std::array<glm::vec2, 128> miplevel;
			for(int l = 1; l < 7; l++)
			{
				int w = (1 << (7 - l));

				for(size_t x = 0; x < miplevel.size(); x++)
					miplevel[x] = glm::vec2(0.0f);

				float scale = float(w) / 128.0f;

				for(size_t x = 0; x < 128; x++)
				{
					double f = x / 127.0;
					size_t slot = size_t((w - 1) * f);
					miplevel[slot] += scale * gauss(f);
				}

				glTextureSubImage1D(
					resources.texFFT,
					l,
					0, w,
					GL_RG,
					GL_FLOAT,
					miplevel.data());
			}
		}
		else
		{
			glGenerateTextureMipmap(resources.texFFT);
		}

		glTextureSubImage2D(
			resources.texChannels,
			0,
			0, 0,
			128, 16,
			GL_RGBA,
			GL_FLOAT,
			channels);
		glGenerateTextureMipmap(resources.texChannels);
	}

    auto const & vis = visualizations.front();
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resources.fb);
        backgroundTexture = vis.resultingImage;

		stageTexture = vis.resultingImage;
		stageDataTexture = 0;

		int stageNum = 0;
		for(auto const & stage : vis.stages)
		{
			glBindTextureUnit(stageTextureSlot, 0);
			glBindTextureUnit(stageDataSlot, 0);

			glNamedFramebufferTexture(
				resources.fb,
				GL_COLOR_ATTACHMENT0,
				stage.renderTarget,
				0);
			glNamedFramebufferTexture(
				resources.fb,
				GL_COLOR_ATTACHMENT1,
				stage.dataTarget,
				0);
			GLenum const drawBuffers[2] =
			{
			    GL_COLOR_ATTACHMENT0,
			    GL_COLOR_ATTACHMENT1
			};
			glNamedFramebufferDrawBuffers(resources.fb, 2, drawBuffers);

			glUseProgram(stage.shader.program);
			{
				const GLuint pgm = stage.shader.program;

				unsigned int textureSlot = 0;
				for(auto const & tuple : stage.shader.uniforms)
				{
					auto const & name = tuple.first;
					auto const & uniform = tuple.second;

					auto resource = vis.resources.find(name);
					auto localResource = stage.shader.resources.find(name);
					auto predefined = uniformMappings.find(name);

					std::vector<MCCTarget> sources;
					transfer(sources, globalCCs, name);
					transfer(sources, vis.ccMapping, name);
					transfer(sources, stage.shader.bindings, name);
					std::sort(
						sources.begin(), sources.end(),
						[](MCCTarget const & lhs, MCCTarget const & rhs)
						{
							return lhs.priority < rhs.priority;
						});

					// first, check for a provided image resource
					if(localResource != stage.shader.resources.end())
					{
						glBindTextureUnit(textureSlot, localResource->second.texture);
						glProgramUniform1i(pgm, uniform.position, int(textureSlot));
						textureSlot++;
					}
					else if(resource != vis.resources.end())
					{
						glBindTextureUnit(textureSlot, resource->second.texture);
						glProgramUniform1i(pgm, uniform.position, int(textureSlot));
						textureSlot++;
					}
					// second, check for a float uniform source
					else if(sources.size() > 0)
					{
						bindCCUniform(pgm, uniform, sources.front());
					}
					// and last, check for a predefined uniform mapping
					else if(predefined != uniformMappings.end())
					{
						predefined->second(stage.shader, uniform, textureSlot);
					}
				}
			}

			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			/*
			char name[256];
			sprintf(name, "/tmp/stage-%d.pgm", stageNum);
			dump_texture(name, stage.renderTarget);

			sprintf(name, "/tmp/data-%d.pgm", stageNum);
			dump_texture(name, stage.dataTarget);
			*/

            stageTexture = stage.renderTarget;
			stageDataTexture = stage.dataTarget;

			stageNum += 1;
		}

		glNamedFramebufferReadBuffer(resources.fb, GL_COLOR_ATTACHMENT0);
		glBlitNamedFramebuffer(
			resources.fb, // src
            0, // dst
            0, 0, width, height, // src-rect
            0, 0, width, height, // dest-rect
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

        if(stageTexture != vis.resultingImage)
		{
			glCopyImageSubData(
                stageTexture, GL_TEXTURE_2D,
				0,
				0, 0, 0,
				vis.resultingImage, GL_TEXTURE_2D,
				0,
				0, 0, 0,
                width, height, 1);
		}

		if(takeScreenshot)
		{
			dump_texture("screenshot.pgm", stageTexture);
			takeScreenshot = false;
		}
	}
    lastFrame = now;
}

static void midiCallback( Midi::Message const & msg)
{
	std::lock_guard<std::mutex> lock(mididataMutex);
	switch(msg.event)
	{
		case Midi::NoteOff:
		case Midi::NoteOn:
			if(msg.payload[1] > 0)
				mididata.channels[msg.channel].notes[msg.payload[0] & 0x7F].attack(msg.payload[1] / 127.0);
			else
				mididata.channels[msg.channel].notes[msg.payload[0] & 0x7F].release();
			break;

		case Midi::ControlChange:
			mididata.channels[msg.channel].ccs[msg.payload[0] & 0x7F] = msg.payload[1] / 127.0;
			mididata.ccs[msg.payload[0] & 0x7F] = mididata.channels[msg.channel].ccs[msg.payload[0] & 0x7F];
			break;

		case Midi::PitchWheelChange: {
			int pitch = (msg.payload[1] << 7) | msg.payload[0];
			auto val = (pitch - 0x2000) / double(0x2000);
			mididata.channels[msg.channel].pitch = val;
			mididata.pitch = val;
			break;
		}
		default: {
			std::stringstream stream;
			for(unsigned int i = 0; i < msg.payload.size(); i++)
			{
				if(i > 0)
					stream << " ";
				stream << std::hex << int(msg.payload[i]) << std::dec;
			}
			Log() << "Unknown MIDI event @" << msg.timestamp << " of length " << msg.payload.size() << ": " << stream.str();
		}
	}
}
