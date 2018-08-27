#include "mshader.hpp"

#include <QFile>
#include <QDebug>

static GLuint getVertexShader()
{
	static GLuint vsh = 0;
	if(vsh != 0)
		return vsh;

	static GLchar const * const vsh_src = R"glsl(#version 330
		const vec2[] vertices = vec2[](
			vec2(0,0),
			vec2(1,0),
			vec2(0,1),
			vec2(1,1)
			);

		out vec2 fUV;

		void main()
		{
			fUV = vertices[gl_VertexID];
			gl_Position = vec4(2.0 * fUV - 1.0, 0.0, 1.0);
		}
		)glsl";

	vsh = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsh, 1, &vsh_src, nullptr);
	glCompileShader(vsh);
	return vsh;
}

MShader::MShader() : program(0), uniforms()
{

}

MShader::MShader(nlohmann::json const & data)
{
	std::vector<QByteArray> strings;
	for(auto const & src : data["sources"])
	{
		QFile file(QString::fromStdString(src.get<std::string>()));
		if(!file.exists())
			qDebug() << "Could not find " << file.fileName();
		file.open(QFile::ReadOnly | QFile::Text);
		auto raw = file.readAll();
		raw.append(char(0));
		strings.emplace_back(raw);
		file.close();
	}

	std::vector<GLchar const *> sources;
	std::transform(
		strings.begin(), strings.end(),
		std::back_inserter(sources),
		[](QByteArray const & data) { return data.data(); });

	GLuint sh = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(sh, GLsizei(sources.size()), sources.data(), nullptr);

	glCompileShader(sh);

	this->program = glCreateProgram();
	glAttachShader(this->program, getVertexShader());
	glAttachShader(this->program, sh);
	glLinkProgram(this->program);

	GLint count;
	glGetProgramiv(this->program, GL_ACTIVE_UNIFORMS, &count);
	for(unsigned int i = 0; i < unsigned(count); i++)
	{
		GLchar buffer[256];
		GLsizei length;
		GLint size;
		GLenum type;

		glGetActiveUniform(this->program, i, sizeof buffer, &length, &size, &type, buffer);

		MUniform uniform;
		uniform.name = QString(QByteArray(buffer, length));
		uniform.position = int(i);
		uniform.type = type;

		this->uniforms.emplace(uniform.name, uniform);
	}
}
