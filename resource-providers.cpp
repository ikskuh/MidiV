#include "mresource.hpp"
#include "debug.hpp"

/******************************************************************************/
#include "providers/noiseprovider.hpp"
/******************************************************************************/

#include <regex>





std::unique_ptr<IResourceProvider> IResourceProvider::get(std::string const & name, nlohmann::json const & data)
{
	if(name == "noise")
		return std::make_unique<NoiseProvider>(data);
	return nullptr;
}
