#ifndef MSHADER_HPP
#define MSHADER_HPP

#include "mcctarget.hpp"

#include <json.hpp>
#include <GL/gl3w.h>

struct MUniform
{
    std::string name;
	int position;
	GLenum type;

	void assertType(GLenum type) const
	{
		if(this->type != type)
			throw std::runtime_error("uniform type mismatch!");
	}
};

struct MShader
{
	GLuint program;
    std::map<std::string, MUniform> uniforms;
	std::map<std::string, MCCTarget> bindings;

	MShader();
	explicit MShader(nlohmann::json const & data);
	explicit MShader(char const * source);
};

#endif // MSHADER_HPP
