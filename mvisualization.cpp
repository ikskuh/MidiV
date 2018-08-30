#include "mvisualization.hpp"

MVisualization::MVisualization(nlohmann::json const & data)
{
    this->title = data["title"].get<std::string>();

	// load all resource
	for(auto const & subdata : data["resources"])
	{
        auto name = subdata["name"].get<std::string>();
		this->resources.emplace(name, MResource(subdata));
	}

	// load all cc mappings
	for(auto const & subdata : data["mappings"])
	{
		auto from = uint8_t(subdata["cc"].get<int>());
        auto to   = subdata["uniform"].get<std::string>();
		this->ccMapping.emplace(from, to);
	}

	// load all shaders
	for(auto const & subdata : data["pipeline"])
	{
        this->stages.emplace_back();
        auto & stage = this->stages.back();
		stage.shader = MShader(subdata);
		stage.renderTarget = 0;
	}
}

void MVisualization::resize(int w, int h)
{
	for(auto & stage : this->stages)
	{
		if(stage.renderTarget != 0)
			glDeleteTextures(1, &stage.renderTarget);
		glCreateTextures(GL_TEXTURE_2D, 1, &stage.renderTarget);
		glTextureStorage2D(
			stage.renderTarget,
			1,
			GL_RGBA32F,
			w, h);
	}
	if(resultingImage != 0)
		glDeleteTextures(1, &resultingImage);

	glCreateTextures(GL_TEXTURE_2D, 1, &this->resultingImage);
	glTextureStorage2D(
		this->resultingImage,
		1,
		GL_RGBA32F,
		w, h);
}
