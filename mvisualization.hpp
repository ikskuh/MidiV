#ifndef MVISUALIZATION_HPP
#define MVISUALIZATION_HPP

#include <QObject>
#include <QList>

#include "mmidistate.hpp"
#include "mshader.hpp"
#include "mresource.hpp"

#include <vector>
#include <map>

struct MRenderStage
{
	MShader shader;
	GLuint renderTarget;
};

struct MVisualization
{
	explicit MVisualization(nlohmann::json const & data);

	void resize(int w, int h);

	QString title;
	std::map<QString,MResource> resources;
	std::map<uint8_t,QString> ccMapping;
	std::vector<MRenderStage> stages;

	GLuint resultingImage;
};

#endif // MVISUALIZATION_HPP
