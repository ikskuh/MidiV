#ifndef MMIDISTATE_HPP
#define MMIDISTATE_HPP

#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>

struct MMidiNote
{
	double value;
	bool isOn;

	void attack(double volume);

	void release();

	void tick(double dt);
};

struct MMidiChannel
{
	uint8_t instrument;
	double pitch;
	double ccs[128];
	MMidiNote notes[128];
};

struct MMidiState
{
	//! global pitch (ignores channel pitches and takes last received event)
	double pitch;

	//! Current state of all channels
	MMidiChannel channels[16];
};

#endif // MMIDISTATE_HPP
