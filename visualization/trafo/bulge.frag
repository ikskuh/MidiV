#version 330

uniform float uBulgeStrength;

uniform float uTime;

uniform ivec2 uScreenSize;

uniform sampler2D uStage;

in vec2 fUV;

out vec4 fragment;

void main()
{
	vec2 xy = 2.0 * fUV - 1.0;
	xy.x *= float(uScreenSize.x) / float(uScreenSize.y);

	xy *= 1.0 -
		uBulgeStrength * pow(max(0, length(xy) - 0.1), 2.0);

	xy.x /= float(uScreenSize.x) / float(uScreenSize.y);

	vec2 uv = 0.5 + 0.5 * xy;

	fragment = texture(uStage, uv);
}
