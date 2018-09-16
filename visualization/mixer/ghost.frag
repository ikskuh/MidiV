#version 330

uniform sampler2D uStage;
uniform sampler2D uBackground;

in vec2 fUV;
out vec4 fragment;

uniform ivec2 uScreenSize;
uniform float uDeltaTime;

uniform float uGhostStrength = 0.95;

vec4 interleave(vec4 now, vec4 then)
{
	return vec4(
		max(now.x, uGhostStrength * then.x),
		max(now.y, uGhostStrength * then.y),
		max(now.z, uGhostStrength * then.z),
		1.0);
}

void main()
{
	vec4 now  = texture(uStage, fUV);
	vec4 then = texture(uBackground, fUV);
	fragment = interleave(now, then);
}
