#pragma once

#include <cstdint>
#include <json.hpp>

struct MCCTarget
{
	enum Type { Unknown, CC, Fixed, Pitch, Note };

	MCCTarget();

	Type type;
	uint8_t channel;
	double priority;
    double scale;
	bool integrate;
	double fixed;
	struct { double low, high; } mapping;

	double value;
	double sum_value;
	uint8_t index;

	bool hasChannel() const { return (this->channel != 0xFF); }

	void update(double deltaTime);

	double get() const
	{
		if(integrate)
			return scale * sum_value;
		else
			return scale * value;
	}

	static MCCTarget load(nlohmann::json const & source);
};
