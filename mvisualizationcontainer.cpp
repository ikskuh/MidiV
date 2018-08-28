#include "mvisualizationcontainer.hpp"

#include <QDebug>
#include <glm/glm.hpp>

using hrc = std::chrono::high_resolution_clock;

MVisualizationContainer::MVisualizationContainer(QWidget *parent) :
    QOpenGLWidget(parent),
    mididata(),
    syncmidi(),
    summidi()
{
	QSurfaceFormat fmt(QSurfaceFormat::DebugContext | QSurfaceFormat::DeprecatedFunctions);
	fmt.setMajorVersion(4);
	fmt.setMinorVersion(5);
	this->setFormat(fmt);

	// make the frames go round!
	connect<void (QOpenGLWidget::*)(), void (QOpenGLWidget::*)()>(
		this, &QOpenGLWidget::frameSwapped,
		this, &QOpenGLWidget::update,
		Qt::AutoConnection);

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


	uniformMappings["uTime"] = [this](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_FLOAT);
		glProgramUniform1f(sh.program, u.position, this->time);
	};
	uniformMappings["uDeltaTime"] = [this](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_FLOAT);
		glProgramUniform1f(sh.program, u.position, this->deltaTime);
	};
	uniformMappings["uScreenSize"] = [this](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_INT_VEC2);
		glProgramUniform2i(sh.program, u.position, this->width, this->height);
	};
	uniformMappings["uMousePos"] = [this](MShader const & sh, MUniform const & u, unsigned int &)
	{
		u.assertType(GL_INT_VEC2);
		glProgramUniform2i(sh.program, u.position, this->mouse.x, this->mouse.y);
	};
	uniformMappings["uBackground"] = [this](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_2D);
		glProgramUniform1i(sh.program, u.position, slot);
		glBindTextureUnit(slot, this->backgroundTexture);
		slot++;
	};
	uniformMappings["uFFT"] = [this](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_1D);
		glProgramUniform1i(sh.program, u.position, slot);
		glBindTextureUnit(slot, resources.texFFT);
		slot++;
	};
	uniformMappings["uChannels"] = [this](MShader const & sh, MUniform const & u, unsigned int & slot)
	{
		u.assertType(GL_SAMPLER_1D_ARRAY);
		glProgramUniform1i(sh.program, u.position, slot);
		glBindTextureUnit(slot, resources.texChannels);
		slot++;
	};

}

void MVisualizationContainer::loadVis(QString const & fileName)
{
	auto current = QDir::currentPath();

	QFile file(fileName);
	if(file.exists())
	{
		file.open(QFile::ReadOnly | QFile::Text);
		auto raw = file.readAll();
		file.close();

		QDir path(fileName);
		path.cdUp();
		QDir::setCurrent(path.path());

		auto data = nlohmann::json::parse(raw.begin(), raw.end());

		this->visualizations.emplace_back(data);
	}
	else
	{
		qDebug() << file.fileName() << "not found!";
	}

	QDir::setCurrent(current);
}

static void nop() { }

static void msglog(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
	(void)source;
	(void)type;
	(void)id;
	(void)severity;
	(void)userParam;
	qDebug() << "[OpenGL]" << QString(QByteArray(message, length));
	if(severity == GL_DEBUG_SEVERITY_HIGH)
		nop();
}

void MVisualizationContainer::initializeGL()
{
	if(gl3wInit() < 0)
		qDebug() << "Failed to initialize GL!";

	glDebugMessageCallback(msglog, nullptr);

	qDebug() << reinterpret_cast<char const *>(glGetString(GL_VERSION));

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

	this->loadVis("visualization/plasmose.vis");

	this->startPoint = hrc::now();
	this->lastFrame = this->startPoint;
}

void MVisualizationContainer::resizeGL(int w, int h)
{
	qDebug() << "Resize to" << w << "×" << h;
	for(auto & vis : this->visualizations)
	{
		vis.resize(w, h);
	}
	this->width = w;
	this->height = h;
}

void MVisualizationContainer::paintGL()
{
	auto now = hrc::now();
	this->time      = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - this->startPoint).count();
	this->deltaTime = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastFrame).count();

	// Clone to prevent changes
	{
		std::lock_guard<std::mutex> lock(this->mididataMutex);

		for(int y = 0; y < 16; y++)
		{
			for(int x = 0; x < 128; x++)
				this->mididata.channels[y].notes[x].tick(double(this->deltaTime));
		}

		this->syncmidi = this->mididata;
	}

	// Integrate midi data
	for(int c = 0; c < 16; c++)
	{
		for(int n = 0; n < 128; n++)
		{
			this->summidi.channels[c].ccs[n] += this->deltaTime * this->syncmidi.channels[c].ccs[n];
			this->summidi.channels[c].notes[n].value += this->deltaTime * this->syncmidi.channels[c].notes[n].value;
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

	auto const & vis = this->visualizations.front();
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resources.fb);
		this->backgroundTexture = vis.resultingImage;
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
						auto mapping = this->uniformMappings.find(name);
						if(mapping != this->uniformMappings.end())
						{
							mapping->second(stage.shader, uniform, textureSlot);
						}
					}
				}
			}

			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			this->backgroundTexture = stage.renderTarget;
		}

		glBlitNamedFramebuffer(
			resources.fb, // src
			this->defaultFramebufferObject(), // dst
			0, 0, this->width, this->height, // src-rect
			0, 0, this->width, this->height, // dest-rect
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		if(this->backgroundTexture != vis.resultingImage)
		{
			glCopyImageSubData(
				this->backgroundTexture, GL_TEXTURE_2D,
				0,
				0, 0, 0,
				vis.resultingImage, GL_TEXTURE_2D,
				0,
				0, 0, 0,
				this->width, this->height, 1);
		}
	}
	this->lastFrame = now;
}


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
			qDebug()
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

void MVisualizationContainer::mousePressEvent(QMouseEvent * event)
{
	this->mouseMoveEvent(event);
}

void MVisualizationContainer::mouseMoveEvent(QMouseEvent * event)
{
	this->mouse = glm::ivec2(event->x(), this->height - event->y() - 1);
	qDebug()
		<< "Mouse Pos: ("
		<< this->mouse.x
	    << this->mouse.y
	    << "), ("
	    << this->mouse.x / double(this->width - 1)
	    << this->mouse.y / double(this->height - 1)
	    << ")"
		;
}
