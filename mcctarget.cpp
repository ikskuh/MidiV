#include "mcctarget.hpp"
#include "debug.hpp"
#include "utils.hpp"

MCCTarget::MCCTarget() : type(Unknown), channel(0xFF), scale(1.0), value(0.0), cc(0)
{
}

MCCTarget MCCTarget::load(nlohmann::json const & override)
{
	auto const src = Utils::get(override, "source", std::string(""));

	MCCTarget target;
	target.channel = uint8_t(Utils::get(override, "channel", 0xFF));
	target.priority = Utils::get(override, "priority", 0.0);
    target.scale = Utils::get(override, "scale", 1.0);

	if((src == "value") || ((src == "") && override.find("value") != override.end()))
	{
		target.type = Fixed;
		target.value = override["value"].get<double>();
	}
	else if((src == "cc") || ((src == "") && override.find("cc") != override.end()))
	{
		target.type = CC;
		target.cc = uint8_t(override["cc"].get<int>());
	}
	else if(src == "pitch")
	{
		target.type = Pitch;
	}
	else
	{
		target.type = Unknown;
		Log() << "Uniform specifier must have either source, cc or value assigned!";
	}
	return target;
}
