#include "faketracker.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include <array>
#include <SDL_ttf.h>
#include <chrono>

static uint8_t fakepack(uint8_t a, uint8_t b)
{
	return ((a ^ (a<<4)) & 0xF0)
	     | ((b ^ (b>>4)) & 0x0F);
}

FakeTracker::FakeTracker(nlohmann::json const & data) :
    texture(0),
    dirty(true),
    target(nullptr, SDL_FreeSurface),
    glyphs(),
    lanes(),
    sync(),
    thread(&FakeTracker::Tick, this)
{
	this->target.reset(SDL_CreateRGBSurfaceWithFormat(
	   0,
	   960, 600,
	   32,
	   SDL_PIXELFORMAT_ARGB8888));
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

	char const * GLYPHS = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-#b";
	for(int i = 0; GLYPHS[i]; i++)
	{
		auto * glyph = TTF_RenderGlyph_Blended(
			font.get(),
			GLYPHS[i],
			SDL_Color { 0x00, 0x00, 0x00, 0xFF });

		this->glyphs.emplace(GLYPHS[i], Utils::sdlptr_t<SDL_Surface>(glyph, SDL_FreeSurface));
	}

	// Fill some "front porch"
	for(int i = 0; i < 64; i++)
	{
		lanes.push_back(lane_t());
	}

	this->hEvent = Midi::RegisterCallback(0x00, [this](Midi::Message const & msg)
	{
		auto & ctx = this->nextLane[msg.channel];
		switch(msg.event)
		{
	    case Midi::ProgramChange:
	        ctx.program = msg.payload[0];
	        break;
		case Midi::NoteOn:
	        ctx.note = msg.payload[0];
	        ctx.vol = msg.payload[1];
	        break;
		case Midi::NoteOff:
			ctx.note = 00;
	        ctx.vol = msg.payload[1];
			break;
		case Midi::ControlChange:
			ctx.cc = fakepack(msg.payload[0], msg.payload[1]);
	        break;
		}
	});

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
	Midi::UnregisterCallback(this->hEvent);
}

static char hexdigit(unsigned int val, int pos)
{
	if(val == 0)
		return ' ';
	val >>= (4 * pos);
	val &= 0xF;
	if(val <= 9)
		return char('0' + val);
	else
		return char('A' + (val - 10));
}

void FakeTracker::update()
{
	std::lock_guard<decltype(sync)> _lock(this->sync);

	SDL_FillRect(
		this->target.get(),
		nullptr,
		0x000000);

	Uint32 brightColumn = SDL_MapRGB(this->target->format, 0x99, 0x99, 0x99);
	Uint32 dimColumn = SDL_MapRGB(this->target->format, 0x77, 0x77, 0x77);

	int const w = this->target->w / 16;
	int const h = 24;

	while((this->lanes.size() * h) > this->target->h)
	{
		this->lanes.erase(this->lanes.begin());
	}

	for(int i = 0; i < 16; i++)
	{
		Uint32 col = (i % 2) ? brightColumn : dimColumn;
		SDL_Rect rect { i * w, 0, w, this->target->h };
		SDL_FillRect(this->target.get(), &rect, col);
	}

	for(size_t i = 0; i < lanes.size(); i++)
	{
		auto const & lane = lanes[i];
		for(size_t j = 0; j < 16; j++)
		{
			auto const & note = lane[j];
			std::array<char, 6> const digits =
			{
			    hexdigit(note.program, 1),
			    hexdigit(note.program, 0),

			    hexdigit(note.note, 1),
			    hexdigit(note.note, 0),

			    hexdigit(note.cc, 1),
			    hexdigit(note.cc, 0),
			};

			SDL_Rect rect = { int(j * w), int(i * h), w, h };
			for(char c : digits)
			{
				SDL_BlitSurface(
					this->glyphs.at(c).get(),
					nullptr,
					this->target.get(),
					&rect);
				rect.x += this->glyphs.at(c)->w;
			}

		}
	}

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

void FakeTracker::Tick(FakeTracker * _this)
{
	using namespace std::chrono;

	auto now = high_resolution_clock::now();
	while(true)
	{
		if(duration_cast<milliseconds>(high_resolution_clock::now() - now).count() > 100)
		{
			std::lock_guard<decltype(sync)> _lock(_this->sync);
			_this->lanes.push_back(_this->nextLane);
			_this->nextLane = lane_t();
			_this->dirty = true;
			now = high_resolution_clock::now();
		}
		/* sleep here */
	}
}
