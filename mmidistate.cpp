#include "mmidistate.hpp"
#include "ccs.hpp"

static constexpr double decayHalfTime = 0.5;
static constexpr double releaseHalfTime = 0.25;
static constexpr double holdValue = 0.3;

void MMidiNote::attack(double volume)
{
	this->value = volume;
	this->isOn = true;
}

void MMidiNote::release()
{
	this->isOn = false;
}

void MMidiNote::tick(double dt)
{
	if(this->isOn)
	{
		this->value *= glm::pow(0.5, dt / decayHalfTime);
		this->value = glm::clamp(this->value, holdValue, 1.0);
	}
	else
	{
		this->value *= glm::pow(0.5, dt / releaseHalfTime);
	}
}

static double cc2dbl(uint8_t value)
{
    return uint8_t(value / 127.0);
}

static void initialize_ccs(std::array<double, 128> & ccs)
{
    ccs[int(CC::Volume       )] = cc2dbl(100);
    ccs[int(CC::Pan          )] = cc2dbl( 64);
    ccs[int(CC::Expression   )] = cc2dbl(127);
    ccs[int(CC::FilterReso   )] = cc2dbl( 64);
    ccs[int(CC::Release      )] = cc2dbl( 64);
    ccs[int(CC::Attack       )] = cc2dbl( 64);
    ccs[int(CC::FilterFreq   )] = cc2dbl( 64);
    ccs[int(CC::Reverb       )] = cc2dbl( 40);
}

MMidiChannel::MMidiChannel() : instrument(0), pitch(0.0), ccs(), notes()
{
    initialize_ccs(this->ccs);
}

MMidiState::MMidiState() : pitch(0.0), ccs(), channels()
{

}
