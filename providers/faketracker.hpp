#pragma once

#include "mresource.hpp"
#include "utils.hpp"
#include "midi.hpp"

#include <SDL.h>
#include <map>
#include <array>
#include <mutex>
#include <thread>

struct FakeTracker :
	public IResourceProvider
{
public:
	GLuint texture;
	volatile bool dirty;

	struct note_t
	{
		note_t() : program(0), note(0), cc(0), vol(0) { }
		uint8_t program;
		uint8_t note;
		uint8_t cc;
		uint8_t vol;
	};

	typedef std::array<note_t, 16> lane_t;

	Utils::sdlptr_t<SDL_Surface> target;
	std::map<char, Utils::sdlptr_t<SDL_Surface>> glyphs;

	std::vector<lane_t> lanes;
	lane_t nextLane;

	Midi::Handle hEvent;
public:
	FakeTracker(nlohmann::json const & data);
	~FakeTracker() override;

public:
	GLuint init() override { return this->texture; }
	bool isDirty() const override { return this->dirty; }
	void update() override;
private:
	std::mutex sync;
	std::thread thread;
	static void Tick(FakeTracker * tracker);
};
