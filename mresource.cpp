#include "mresource.hpp"
#include <stb_image.h>
#include "debug.hpp"
#include "utils.hpp"
#include <memory>

MResource::MResource() : name(), texture(0)
{

}

MResource::MResource(nlohmann::json const & data)
{
	using Utils::get;

    this->name = data["name"].get<std::string>();
	auto type = data["type"].get<std::string>();

	// try to get a resource provider
	// or get "nullptr" for no resource provider found.
	this->provider = IResourceProvider::get(type, data);

	if(provider != nullptr)
	{
		this->texture = this->provider->init();
	}
	else
	{
		int width = 0;
		int height = 0;
		int depth = 0;
		int slices = 0;
		stbi_uc * imgdata = nullptr;

		// preload all shared parameters
		width = get(data, "width", 0);
		height = get(data, "height", 0);
		depth = get(data, "depth", 0);
		slices = get(data, "slices", 0);

		// preload image file if any
		if(data.find("file") != data.end())
		{
			std::string fileName = data["file"].get<std::string>();

			imgdata = stbi_load(fileName.c_str(), &width, &height, nullptr, 4);
			if(imgdata == nullptr)
				Log() << "Could not load " << fileName.c_str();
		}

		// read mipmap property
		int layers = int(1 + std::log2(std::max({ width, height, depth })));
		if(get(data, "mipmaps", true) == false)
			layers = 1;

		auto const texformat = GL_RGBA8;
		auto const dataformat = GL_RGBA;
		auto const datatype = GL_UNSIGNED_BYTE;

		if(type == "sampler1D")
		{
			glCreateTextures(GL_TEXTURE_1D, 1, &this->texture);
			glTextureStorage1D(
				this->texture,
				layers,
				texformat,
				width);
			if(imgdata != nullptr)
			{
				glTextureSubImage1D(
					this->texture,
					0,
					0, width,
					dataformat,
					datatype,
					imgdata);
			}
		}
		else if(type == "sampler1DArray")
		{
			glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &this->texture);
			glTextureStorage2D(
				this->texture,
				layers,
				texformat,
				width, slices);
			if(imgdata != nullptr)
			{
				glTextureSubImage2D(
					this->texture,
					0,
					0, 0,
					width, slices,
					dataformat,
					datatype,
					imgdata);
			}
		}
		else if(type == "sampler2D")
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);
			glTextureStorage2D(
				this->texture,
				layers,
				texformat,
				width, height);
			if(imgdata != nullptr)
			{
				glTextureSubImage2D(
					this->texture,
					0,
					0, 0,
					width, height,
					dataformat,
					datatype,
					imgdata);
			}

		}
		else if(type == "sampler2DArray")
		{
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &this->texture);
			glTextureStorage3D(
				this->texture,
				layers,
				texformat,
				width, height, slices);
			if(imgdata != nullptr)
			{
				glTextureSubImage3D(
					this->texture,
					0,
					0, 0, 0,
					width, height, slices,
					dataformat,
					datatype,
					imgdata);
			}
		}
		else if(type == "sampler3D")
		{
			glCreateTextures(GL_TEXTURE_3D, 1, &this->texture);
			glTextureStorage3D(
				this->texture,
				layers,
				texformat,
				width, height, depth);
			if(imgdata != nullptr)
			{
				glTextureSubImage3D(
					this->texture,
					0,
					0, 0, 0,
					width, height, depth,
					dataformat,
					datatype,
					imgdata);
			}
		}
		else if(type == "samplerCUBE")
		{
			glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &this->texture);

			// fix image size
			slices = 6;
			if(imgdata != nullptr)
				height /= 6;

			glTextureStorage2D(
				this->texture,
				layers,
				texformat,
				width, height);
			if(imgdata != nullptr)
			{
				glTextureSubImage3D(
					this->texture,
					0,
					0, 0, 0,
					width, height, slices,
					dataformat,
					datatype,
					imgdata);
			}
		}

		auto filter = get(data, "filter", std::string("linear"));

		auto const nonmipfilter = (filter == "linear") ? GL_LINEAR : GL_NEAREST;
		auto const mipfilter = (filter == "linear") ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;


		GLint texwrap = (type == "samplerCUBE") ? GL_CLAMP_TO_EDGE : GL_REPEAT;
		auto wrap = get(data, "wrap", std::string("repeat"));
		if(wrap == "clamp")
			texwrap = GL_CLAMP_TO_EDGE;
		else if(wrap == "repeat")
			texwrap = GL_REPEAT;
		else if(wrap == "mirror-clamp")
			texwrap = GL_MIRROR_CLAMP_TO_EDGE;
		else if(wrap == "mirror-repeat")
			texwrap = GL_MIRRORED_REPEAT;
		else if(wrap == "border")
			texwrap = GL_CLAMP_TO_BORDER;
		else
			Log() << "Unknown texture wrap" << wrap.c_str();

		glTextureParameteri(this->texture, GL_TEXTURE_MAG_FILTER, (filter == "linear") ? GL_LINEAR : GL_NEAREST);
		glTextureParameteri(this->texture, GL_TEXTURE_MIN_FILTER, (layers > 1) ? mipfilter : nonmipfilter);

		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_S, texwrap);
		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_T, texwrap);
		glTextureParameteri(this->texture, GL_TEXTURE_WRAP_R, texwrap);

		// Calculate mipmaps if necessary
		if(layers > 1)
			glGenerateTextureMipmap(this->texture);

		if(imgdata != nullptr)
			free(imgdata);
	}
}

void MResource::update()
{
	if(this->provider && this->provider->isDirty())
		this->provider->update();
}
