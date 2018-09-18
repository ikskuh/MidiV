#pragma once

#include "mresource.hpp"

#include <vector>
#include <cstdint>

struct NoiseProvider :
	public IResourceProvider
{
	GLuint texture;
	bool dynamic;
	std::vector<uint8_t> data;
	int size;

	explicit NoiseProvider(nlohmann::json const & data);
	~NoiseProvider() override;

	GLuint init() override { return this->texture; }
	void update() override;
	bool isDirty() const override { return this->dynamic; }
};
