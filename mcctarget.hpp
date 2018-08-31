#pragma once

#include <cstdint>
#include <json.hpp>

struct MCCTarget
{
	enum Type { Unknown, CC, Fixed };

	Type type;
	uint8_t value;
	uint8_t channel;
	uint8_t cc;

	static MCCTarget load(nlohmann::json const & source);
};
