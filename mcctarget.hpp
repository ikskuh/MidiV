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

	double value;
	uint8_t index;

	bool hasChannel() const { return (this->channel != 0xFF); }

	static MCCTarget load(nlohmann::json const & source);
};
