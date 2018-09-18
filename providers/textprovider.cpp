#include "textprovider.hpp"
#include "debug.hpp"
#include "utils.hpp"

#include <SDL_ttf.h>

TextProvider::TextProvider(nlohmann::json const & data)
{
	SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };

	auto text      = Utils::get(data, "text", std::string(""));
	auto fontFile  = Utils::get(data, "font", std::string(""));
	auto size      = Utils::get(data, "size", 16);
	auto colorSpec = Utils::get(data, "color", std::string("#000000"));

	if((colorSpec.size() > 0) && (colorSpec[0] == '#'))
	{
		color.r = std::strtol(colorSpec.substr(1, 2).c_str(), nullptr, 16);
		color.g = std::strtol(colorSpec.substr(3, 2).c_str(), nullptr, 16);
		color.b = std::strtol(colorSpec.substr(5, 2).c_str(), nullptr, 16);
	}

	std::unique_ptr<TTF_Font, void(*)(TTF_Font*)> font(TTF_OpenFont(fontFile.c_str(), size), TTF_CloseFont);
	if(font == nullptr)
	{
		Log() << "Could not open font '" << fontFile << "'.";
		Utils::FlagError();
		return;
	}

	std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> img_unknown(TTF_RenderUTF8_Blended(font.get(), text.c_str(), color), SDL_FreeSurface);
	if(img_unknown == nullptr)
	{
		Log() << "Could not render string '" << text << "'.";
		Utils::FlagError();
		return;
	}

	std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> img(SDL_ConvertSurfaceFormat(
		img_unknown.get(),
		SDL_PIXELFORMAT_ARGB8888,
		0),
		SDL_FreeSurface);
	if(img == nullptr)
	{
		Log() << "Could not convert image to RGBA8888 format.";
		Utils::FlagError();
		return;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &this->texture);
	glTextureStorage2D(
		this->texture,
		1,
		GL_RGBA8,
		img->w, img->h);

	if(SDL_MUSTLOCK(img.get()))
		SDL_LockSurface(img.get());

	glTextureSubImage2D(
		this->texture,
		0,
		0, 0,
		img->w, img->h,
		GL_BGRA,
		GL_UNSIGNED_BYTE,
		img->pixels);

	if(SDL_MUSTLOCK(img.get()))
		SDL_UnlockSurface(img.get());

}

TextProvider::~TextProvider()
{
	glDeleteTextures(1, &this->texture);
}
