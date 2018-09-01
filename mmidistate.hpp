#ifndef MMIDISTATE_HPP
#define MMIDISTATE_HPP

#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include <array>

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
    MMidiChannel();

	uint8_t instrument;
    double pitch;
    std::array<double, 128> ccs;
    std::array<MMidiNote, 128> notes;
};

struct MMidiState
{
    MMidiState();

	//! global pitch (ignores channel pitches and takes last received event)
	double pitch;

	//! global CCs (ignores channels and takes last received event for value)
    std::array<double, 128> ccs;

	//! Current state of all channels
    std::array<MMidiChannel, 16> channels;
};

#endif // MMIDISTATE_HPP
