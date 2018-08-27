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

private slots:
	Q_SLOT void sequencerEvent( drumstick::SequencerEvent* ev );

private:
	void loadVis(QString const & fileName);

private:
	drumstick::MidiClient * client;
	drumstick::MidiPort * port;

	std::mutex mididataMutex;
	MMidiState mididata, syncmidi;

	std::vector<MVisualization> visualizations;
	std::chrono::high_resolution_clock::time_point startPoint, lastFrame;

	std::map<QString, std::function<void(MShader const &, MUniform const &, unsigned int &)>> uniformMappings;

	int width, height;

	float time, deltaTime;

	GLuint backgroundTexture;
};

#endif // MVISUALIZATIONCONTAINER_HPP
