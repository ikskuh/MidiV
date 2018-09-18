#pragma once

#include "mresource.hpp"
#include "utils.hpp"
#include <SDL.h>
#include <map>

struct FakeTracker :
	public IResourceProvider
{
public:
	GLuint texture;
	volatile bool dirty;

	Utils::sdlptr_t<SDL_Surface> target;
	std::map<char, Utils::sdlptr_t<SDL_Surface>> glyphs;
public:
	FakeTracker(nlohmann::json const & data);
	~FakeTracker() override;

public:
	GLuint init() override { return this->texture; }
	bool isDirty() const override { return this->dirty; }
	void update() override;
};
