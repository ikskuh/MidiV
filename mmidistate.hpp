#ifndef MMIDISTATE_HPP
#define MMIDISTATE_HPP

#include <cstdint>

struct MMidiChannel
{
	uint8_t instrument;
	uint8_t ccs[128];
	uint8_t notes[128];
};

struct MMidiState
{
	//! Time since start of visualization
	double time;

	//! Current state of all channels
	MMidiChannel channels[16];
};

#endif // MMIDISTATE_HPP
