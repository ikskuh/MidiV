#include "mvisualization.hpp"
#include "utils.hpp"

MVisualization::MVisualization(nlohmann::json const & data)
{
    this->title = data["title"].get<std::string>();

	// load all resource
	for(auto const & subdata : Utils::get(data, "resources"))
	{
        auto name = subdata["name"].get<std::string>();
		this->resources.emplace(name, MResource(subdata));
	}

	// load all cc mappings
	for(auto const & subdata : Utils::get(data, "bindings"))
	{
		auto from = subdata["uniform"].get<std::string>();
		auto target = MCCTarget::load(subdata);
		this->ccMapping.emplace(from, target);
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
	GLenum constexpr imageFormat = GL_RGBA8;
	GLenum constexpr dataFormat = GL_RGBA32F;

	for(auto & stage : this->stages)
	{
		if(stage.renderTarget != 0)
			glDeleteTextures(1, &stage.renderTarget);

		if(stage.dataTarget != 0)
			glDeleteTextures(1, &stage.dataTarget);

		glCreateTextures(GL_TEXTURE_2D, 1, &stage.renderTarget);
		glTextureStorage2D(
			stage.renderTarget,
			1,
			imageFormat,
			w, h);

		glCreateTextures(GL_TEXTURE_2D, 1, &stage.dataTarget);
		glTextureStorage2D(
			stage.dataTarget,
			1,
			dataFormat,
			w, h);
	}
	if(resultingImage != 0)
		glDeleteTextures(1, &resultingImage);

	glCreateTextures(GL_TEXTURE_2D, 1, &this->resultingImage);
	glTextureStorage2D(
		this->resultingImage,
		1,
		imageFormat,
		w, h);
}
