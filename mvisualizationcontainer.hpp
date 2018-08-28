#ifndef MVISUALIZATIONCONTAINER_HPP
#define MVISUALIZATIONCONTAINER_HPP

#include <vector>
#include <GL/gl3w.h>
#include <QOpenGLWidget>
#include <chrono>

#include <drumstick.h>
#undef marker

#include <mutex>
#include "mvisualization.hpp"

#include <glm/glm.hpp>

class MVisualizationContainer :
	public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit MVisualizationContainer(QWidget *parent = nullptr);
	virtual ~MVisualizationContainer() = default;

	void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
	Q_SLOT void sequencerEvent( drumstick::SequencerEvent* ev );

	void mousePressEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;

private:
	void loadVis(QString const & fileName);

private:
	drumstick::MidiClient * client;
	drumstick::MidiPort * port;

	std::mutex mididataMutex;
	// current state, synchronized state, summed state
	MMidiState mididata, syncmidi, summidi;

	std::vector<MVisualization> visualizations;
	std::chrono::high_resolution_clock::time_point startPoint, lastFrame;

	std::map<QString, std::function<void(MShader const &, MUniform const &, unsigned int &)>> uniformMappings;

	int width, height;

	float time, deltaTime;

	GLuint backgroundTexture;

	glm::vec2 mouse;

	struct
	{
		GLuint texFFT, texChannels;
		GLuint vao, fb;
	} resources;
};

#endif // MVISUALIZATIONCONTAINER_HPP
