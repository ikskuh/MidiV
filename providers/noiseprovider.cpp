#include "noiseprovider.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include <random>

static std::uniform_int_distribution<unsigned int> distribution(0, 255);
static std::mt19937 randengine;

NoiseProvider::NoiseProvider(nlohmann::json const & data)
{
	glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);

	this->dynamic = Utils::get(data, "dynamic", false);

	this->size = Utils::get(data, "size", 128);
	if(this->size <= 0)
	{
		Log() << "Noise size must be greater than zero!";
		Utils::FlagError();
		return;
	}

	this->data.resize(4 * size_t(size) * size_t(size));

	glTextureStorage2D(
		this->texture,
		1,
		GL_RGBA8,
		size, size);

	this->update();
}

NoiseProvider::~NoiseProvider()
{
	glDeleteTextures(1, &this->texture);
}

void NoiseProvider::update()
{
	for(auto & val : this->data)
		val = uint8_t(distribution(randengine));
	glTextureSubImage2D(
		this->texture,
		0,
		0, 0,
		this->size, this->size,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		this->data.data());
}
