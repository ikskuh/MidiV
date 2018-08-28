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
	double ccs[128];
	MMidiNote notes[128];
};

struct MMidiState
{
	//! Time since start of visualization
	double time;

	//! Current state of all channels
	MMidiChannel channels[16];
};

#endif // MMIDISTATE_HPP
