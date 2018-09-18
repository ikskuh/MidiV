#pragma once

#include "mresource.hpp"

struct TextProvider :
	public IResourceProvider
{
	GLuint texture;

public:
	TextProvider(nlohmann::json const & data);
	~TextProvider() override;

public:
	GLuint init() override { return this->texture; }

	bool isDirty() const override { return false; }

	void update() override { /* snip */ }
};
