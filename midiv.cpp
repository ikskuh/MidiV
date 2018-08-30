#include "midiv.hpp"
#include "hal.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include <glm/glm.hpp>
#include <fstream>

using hrc = std::chrono::high_resolution_clock;


#ifdef MIDIV_LINUX
static drumstick::MidiClient * client;
static drumstick::MidiPort * port;
#endif

static std::mutex mididataMutex;
// current state, synchronized state, summed state
static MMidiState mididata, syncmidi, summidi;

static std::vector<MVisualization> visualizations;
static std::chrono::high_resolution_clock::time_point startPoint, lastFrame;

static std::map<std::string, std::function<void(MShader const &, MUniform const &, unsigned int &)>> uniformMappings;

static int width, height;

static float totalTime, deltaTime;

static GLuint backgroundTexture;

static glm::ivec2 mouse;

static struct
{
    GLuint texFFT, texChannels;
    GLuint vao, fb;
} resources;

static void loadVis(std::string const & fileName);

static void nop() { }


void APIENTRY msglog(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    (void)source;
    (void)type;
    (void)id;
    (void)severity;
    (void)userParam;
    fprintf(stderr, "[OpenGL] ");
    fwrite(message, length, 1, stderr);
    fprintf(stderr, "\n");
    fflush(stderr);

    if(severity == GL_DEBUG_SEVERITY_HIGH)
        nop(); // point to break on
}

void MidiV::Initialize()
{
#ifdef MIDIV_LINUX
	client = new drumstick::MidiClient(this);
    client->open();
    client->setClientName("Midi-V");
    connect(
		client, &drumstick::MidiClient::eventReceived,
		this,   &MVisualizationContainer::sequencerEvent,
		Qt::DirectConnection );
    port = new drumstick::MidiPort(this);
    port->attach( client );
    port->setPortName("Midi-V Port");
    port->setCapability( SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
    port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
	port->subscribeFromAnnounce();

	drumstick::PortInfo info(client, 14, 0);
	port->subscribeFrom(&info);

	client->setRealTimeInput(false);
    client->startSequencerInput();
#endif


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
    }



    glCreateFramebuffers(1, &resources.fb);

    loadVis("visualization/plasmose.vis");

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
    uniformMappings["uScreenSize"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_INT_VEC2);
        glProgramUniform2i(sh.program, u.position, width, height);
	};
    uniformMappings["uMousePos"] = [](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_INT_VEC2);
        glProgramUniform2i(sh.program, u.position, mouse.x, mouse.y);
	};
    uniformMappings["uBackground"] = [](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_2D);
		glProgramUniform1i(sh.program, u.position, slot);
        glBindTextureUnit(slot, backgroundTexture);
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
}

static void loadVis(std::string const & fileName)
{
    auto current = HAL::GetWorkingDirectory();

    Log() << "do i get here?";

    auto file = Utils::LoadFile<char>(fileName);

    if(file)
    {
        ptrdiff_t offset;
        auto fullPath = HAL::GetFullPath(fileName, &offset);

        Log() << "A" << fullPath << offset;
        fullPath = fullPath.substr(0, offset);
        Log() << "B" << fullPath;

        // QDir path(fileName);
        // path.cdUp();
        // QDir::setCurrent(path.path());

        auto data = nlohmann::json::parse(file->begin(), file->end());

        visualizations.emplace_back(data);
	}
	else
	{
        Log() << fileName << "not found!";
	}

    HAL::SetWorkingDirectory(current);
}

void MidiV::Resize(int w, int h)
{
    Log() << "Resize to" << w << "×" << h;
    for(auto & vis : visualizations)
	{
		vis.resize(w, h);
	}
    width = w;
    height = h;
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

	// Integrate midi data
	for(int c = 0; c < 16; c++)
	{
		for(int n = 0; n < 128; n++)
		{
            summidi.channels[c].ccs[n] += deltaTime * syncmidi.channels[c].ccs[n];
            summidi.channels[c].notes[n].value += deltaTime * syncmidi.channels[c].notes[n].value;
		}
	}


	{
		glm::vec2 fft[128];
		glm::vec4 channels[16][128];

		for(int x = 0; x < 128; x++)
		{
			fft[x] = glm::vec2(0);
			for(int y = 0; y < 16; y++)
			{
				if(y != 9)
				{
					fft[x].x += syncmidi.channels[y].notes[x].value;
					fft[x].y += summidi.channels[y].notes[x].value;
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
			fft);
		glGenerateTextureMipmap(resources.texFFT);

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
		for(auto const & stage : vis.stages)
		{
			glNamedFramebufferTexture(
				resources.fb,
				GL_COLOR_ATTACHMENT0,
				stage.renderTarget,
				0);
			glNamedFramebufferDrawBuffer(resources.fb, GL_COLOR_ATTACHMENT0);

			glUseProgram(stage.shader.program);
			{
				unsigned int textureSlot = 0;
				for(auto const & tuple : stage.shader.uniforms)
				{
					auto const & name = tuple.first;
					auto const & uniform = tuple.second;

					auto resource = vis.resources.find(name);
					if(resource != vis.resources.end())
					{
						glBindTextureUnit(textureSlot, resource->second.texture);
						glProgramUniform1i(stage.shader.program, uniform.position, textureSlot);
						textureSlot++;
					}
					else
					{
                        auto mapping = uniformMappings.find(name);
                        if(mapping != uniformMappings.end())
						{
							mapping->second(stage.shader, uniform, textureSlot);
						}
					}
				}
			}

			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            backgroundTexture = stage.renderTarget;
		}

		glBlitNamedFramebuffer(
			resources.fb, // src
            0, // dst
            0, 0, width, height, // src-rect
            0, 0, width, height, // dest-rect
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

        if(backgroundTexture != vis.resultingImage)
		{
			glCopyImageSubData(
                backgroundTexture, GL_TEXTURE_2D,
				0,
				0, 0, 0,
				vis.resultingImage, GL_TEXTURE_2D,
				0,
				0, 0, 0,
                width, height, 1);
		}
	}
    lastFrame = now;
}


#ifdef MIDIV_LINUX
void MVisualizationContainer::sequencerEvent( drumstick::SequencerEvent* ev )
{
	using namespace drumstick;

	std::lock_guard<std::mutex> lock(this->mididataMutex);

	switch(ev->getSequencerType())
	{
		case SND_SEQ_EVENT_NOTEON: {
			auto * e = static_cast<NoteOnEvent*>(ev);
			if(e->getVelocity() > 0)
				this->mididata.channels[e->getChannel()].notes[e->getKey()].attack(e->getVelocity() / 127.0);
			else
				this->mididata.channels[e->getChannel()].notes[e->getKey()].release();
			break;
		}
		case SND_SEQ_EVENT_NOTEOFF: {
			auto * e = static_cast<NoteOffEvent*>(ev);
			this->mididata.channels[e->getChannel()].notes[e->getKey()].release();
			break;
		}
		case SND_SEQ_EVENT_PGMCHANGE: {
			auto *e = static_cast<ProgramChangeEvent*>(ev);
            Log()
				<< "Program Change"
				<< e->getChannel()
				<< e->getValue()
				;
			this->mididata.channels[e->getChannel()].instrument = uint8_t(e->getValue());
			break;
		}
		case SND_SEQ_EVENT_CONTROL14:
	    case SND_SEQ_EVENT_NONREGPARAM:
	    case SND_SEQ_EVENT_REGPARAM:
	    case SND_SEQ_EVENT_CONTROLLER:
		{
	        auto * e = static_cast<ControllerEvent*>(ev);
			this->mididata.channels[e->getChannel()].ccs[e->getParam()] = e->getValue() / 127.0;
	        break;
	    }
	}
	delete ev;
}
#endif

/*
void MVisualizationContainer::mousePressEvent(QMouseEvent * event)
{
	this->mouseMoveEvent(event);
}

void MVisualizationContainer::mouseMoveEvent(QMouseEvent * event)
{
	this->mouse = glm::ivec2(event->x(), this->height - event->y() - 1);
    Log()
		<< "Mouse Pos: ("
		<< this->mouse.x
	    << this->mouse.y
	    << "), ("
	    << this->mouse.x / double(this->width - 1)
	    << this->mouse.y / double(this->height - 1)
	    << ")"
		;
}
*/
