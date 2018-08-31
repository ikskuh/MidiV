#ifndef MVISUALIZATION_HPP
#define MVISUALIZATION_HPP


#include "mmidistate.hpp"
#include "mshader.hpp"
#include "mresource.hpp"

#include <string>
#include <vector>
#include <map>

struct MRenderStage
{
	MShader shader;
	GLuint renderTarget;
};

struct MCCTarget
{
	uint8_t channel;
	uint8_t cc;
};

struct MVisualization
{
	MVisualization() = default;
	explicit MVisualization(nlohmann::json const & data);

	void resize(int w, int h);

    std::string title;
    std::map<std::string,MResource> resources;
    std::map<std::string, MCCTarget> ccMapping;
	std::vector<MRenderStage> stages;

	GLuint resultingImage;
};

#endif // MVISUALIZATION_HPP
