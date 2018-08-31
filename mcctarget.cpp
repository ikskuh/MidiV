#include "mcctarget.hpp"
#include "debug.hpp"
#include "utils.hpp"

MCCTarget MCCTarget::load(nlohmann::json const & override)
{
	MCCTarget target;
	if(override.find("value") != override.end())
	{
		target.type = Fixed;
		target.value = override["value"].get<double>();
	}
	else if(override.find("cc") != override.end())
	{
		target.type = CC;
		target.cc = uint8_t(override["cc"].get<int>());
		target.channel = uint8_t(Utils::get(override, "channel", 0));
	}
	else
	{
		target.type = Unknown;
		Log() << "Uniform specifier must have either cc or value assigned!";
	}
	return target;
}
