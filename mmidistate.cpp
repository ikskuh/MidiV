#include "mmidistate.hpp"

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
