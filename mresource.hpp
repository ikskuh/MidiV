#ifndef MRESOURCE_HPP
#define MRESOURCE_HPP

#include <GL/gl3w.h>
#include <functional>
#include <string>
#include <json.hpp>
#include "mmidistate.hpp"

//! A resource containing a texture of a certain type.
//!
//! ```
//! {
//!		"name": "rGradient",
//!		"type": "sampler1D",
//!		"file": "plasma-gradient.png"
//! }
//! ```
struct MResource
{
    std::string name;
	GLuint texture;

	// creates a null resource
	MResource();

	//! loads the resource from a given chunk of json data.
	explicit MResource(nlohmann::json const & data);
};

#endif // MRESOURCE_HPP
