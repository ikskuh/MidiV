#include "midiv.hpp"
#include "hal.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <array>

#include <RtMidi.h>

using hrc = std::chrono::high_resolution_clock;

static std::mutex mididataMutex;
// current state, synchronized state, summed state
static MMidiState mididata, syncmidi, summidi;

static std::array<MVisualization, 128> visualizations;
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

static std::unique_ptr<RtMidiIn> midi;

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

static void midiCallback( double timeStamp, std::vector<unsigned char> *message, void *userData);

void MidiV::Initialize()
{
	midi = std::make_unique<RtMidiIn>(RtMidi::UNSPECIFIED, "Midi-V");
	midi->setCallback(midiCallback, nullptr);
	midi->openVirtualPort("Visualization Input");
	midi->ignoreTypes(); // no time, no sense, no sysex

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
    Log() << "Resize to " << w << "×" << h;
	glViewport(0, 0, w, h);
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

/*
1000nnnn	0kkkkkkk	0vvvvvvv	Note Off event.
									This message is sent when a note is released (ended).
									(kkkkkkk) is the key (note) number.
									(vvvvvvv) is the velocity.
1001nnnn	0kkkkkkk	0vvvvvvv	Note On event.
									This message is sent when a note is depressed (start).
									(kkkkkkk) is the key (note) number.
									(vvvvvvv) is the velocity.
1010nnnn	0kkkkkkk	0vvvvvvv	Polyphonic Key Pressure (Aftertouch).
									This message is most often sent by pressing down on the key after it "bottoms out".
									(kkkkkkk) is the key (note) number.
									(vvvvvvv) is the pressure value.
1011nnnn	0ccccccc	0vvvvvvv	Control Change.
									This message is sent when a controller value changes. Controllers include devices such as pedals and levers. Certain controller numbers are reserved for specific purposes. See Channel Mode Messages.
									(ccccccc) is the controller number.
									(vvvvvvv) is the new value.
1100nnnn	0ppppppp				Program Change.
									This message sent when the patch number changes.
									(ppppppp) is the new program number.
1101nnnn	0vvvvvvv				Channel Pressure (After-touch).
									This message is most often sent by pressing down on the key after it "bottoms out". This message is different from polyphonic after-touch. Use this message to send the single greatest pressure value (of all the current depressed keys).
									(vvvvvvv) is the pressure value.
1110nnnn	0lllllll	0mmmmmmm	Pitch Wheel Change.
									This message is sent to indicate a change in the pitch wheel. The pitch wheel is measured by a fourteen bit value. Centre (no pitch change) is 2000H. Sensitivity is a function of the transmitter.
									(lllllll) are the least significant 7 bits.
									(mmmmmmm) are the most significant 7 bits.
*/

struct MidiMsg
{
	static constexpr uint8_t NoteOff          = 0x80;
	static constexpr uint8_t NoteOn           = 0x90;
	static constexpr uint8_t Aftertouch       = 0xA0;
	static constexpr uint8_t ControlChange    = 0xB0;
	static constexpr uint8_t ProgramChange    = 0xC0;
	static constexpr uint8_t ChannelPressure  = 0xD0;
	static constexpr uint8_t PitchWheelChange = 0xE0;
	static constexpr uint8_t MASK             = 0xF0;
};

void midiCallback( double timeStamp, std::vector<unsigned char> * message, void * /*userData*/)
{
	std::lock_guard<std::mutex> lock(mididataMutex);

	auto chan = message->at(0) & 0x0F;
	switch(message->at(0) & MidiMsg::MASK)
	{
		case MidiMsg::NoteOff:
		case MidiMsg::NoteOn:
			if(message->at(2) > 0)
				mididata.channels[chan].notes[message->at(1) & 0x7F].attack(message->at(2) / 127.0);
			else
				mididata.channels[chan].notes[message->at(1) & 0x7F].release();
			break;
		default: {
			std::stringstream stream;
			for(unsigned int i = 0; i < message->size(); i++)
			{
				if(i > 0)
					stream << " ";
				stream << std::hex << int(message->at(i)) << std::dec;
			}
			Log() << "Unknown MIDI event @" << timeStamp << " of length " << message->size() << ": " << stream.str();
		}
	}

}

/*
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
*/

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
