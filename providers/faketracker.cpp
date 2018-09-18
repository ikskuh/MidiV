#include "faketracker.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include <array>
#include <SDL_ttf.h>

FakeTracker::FakeTracker(nlohmann::json const & data) :
    texture(0),
    dirty(true),
    target(nullptr, SDL_FreeSurface),
    glyphs()
{
	this->target.reset(SDL_CreateRGBSurfaceWithFormat(
	   0,
	   800, 600,
	   32,
	   SDL_PIXELFORMAT_RGBA8888));
	if(this->target == nullptr)
	{
		Log() << SDL_GetError();
		Utils::FlagError();
		return;
	}

	std::unique_ptr<TTF_Font, void(*)(TTF_Font*)> font(
		TTF_OpenFont("fonts/DroidSansMono.ttf", 16),
		TTF_CloseFont);
	if(font == nullptr)
	{
		Log() << SDL_GetError();
		Utils::FlagError();
		return;
	}

	char const * GLYPHS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-#b";
	for(int i = 0; GLYPHS[i]; i++)
	{
		auto * glyph = TTF_RenderGlyph_Blended(
			font.get(),
			GLYPHS[i],
			SDL_Color { 0x00, 0x00, 0x00, 0xFF });

		this->glyphs.emplace(GLYPHS[i], Utils::sdlptr_t<SDL_Surface>(glyph, SDL_FreeSurface));
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);
	glTextureStorage2D(
		this->texture,
		1,
		GL_RGBA8,
		this->target->w, this->target->h);
	this->update();
}

FakeTracker::~FakeTracker()
{
	glDeleteTextures(1, &this->texture);
}

void FakeTracker::update()
{
	SDL_FillRect(
		this->target.get(),
		nullptr,
		0xFF00FF);

	SDL_BlitSurface(
		this->glyphs.at('L').get(),
		nullptr,
		this->target.get(),
		nullptr);

	if(SDL_MUSTLOCK(this->target))
		SDL_LockSurface(this->target.get());

	glTextureSubImage2D(
		this->texture,
		0,
		0, 0,
		this->target->w, this->target->h,
		GL_BGRA,
		GL_UNSIGNED_BYTE,
		this->target->pixels);

	if(SDL_MUSTLOCK(this->target))
		SDL_UnlockSurface(this->target.get());
	this->dirty = false;
}
