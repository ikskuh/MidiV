#ifndef MRESOURCE_HPP
#define MRESOURCE_HPP

#include <GL/gl3w.h>
#include <functional>
#include <string>
#include <json.hpp>
#include "mmidistate.hpp"

struct IResourceProvider
{
public:
	virtual ~IResourceProvider() = default;

public:
	//! initializes the resource provider and returns an
	//! OpenGL texture object id.
	virtual GLuint init() = 0;

	//! returns true if the resource provider has new data
	virtual bool isDirty() const = 0;

	//! updates the resource returned by init() and resets the dirty flag.
	virtual void update() = 0;

public:
	//! Creates a new resource provider from the given name and configuration.
	//! @returns nullptr when no matching resource provider was found or pointer to the provider.
	static std::unique_ptr<IResourceProvider> get(std::string const & name, nlohmann::json const & data);
};

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
	std::unique_ptr<IResourceProvider> provider;

	// creates a null resource
	MResource();

	//! loads the resource from a given chunk of json data.
	explicit MResource(nlohmann::json const & data);

	//! updates the resource if it has a provider and the provider is dirty.
	void update();

};

#endif // MRESOURCE_HPP
