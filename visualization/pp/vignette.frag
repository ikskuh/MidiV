#version 330
/*
{
	"sources": [ "vignette.frag" ],
	"bindings": [
		{ "uniform": "uVignetteSize", "value": 0.8 }
	]
}
*/

uniform sampler2D uStage;

uniform ivec2 uScreenSize;

uniform float uTime;

uniform float uVignetteSize;

in vec2 fUV;

layout(location = 0) out vec4 fragment;

void main()
{
	fragment = texture(uStage, fUV);

	vec2 xy = abs(2.0 * fUV - 1.0);

	fragment = mix(
		fragment,
		vec4(0, 0, 0, 1),
		clamp((1.0 / uVignetteSize) * length(xy) - uVignetteSize, 0.0, 1.0));
}
