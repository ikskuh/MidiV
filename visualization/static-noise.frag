#version 330
/*
{
	"sources": [ "static-noise.frag" ],
	"bindings": [
		{ "uniform": "uStaticNoiseStrength", "value": 0.1 },
		{ "uniform": "uStaticNoiseGrainSize", "value": 2 }
	]
}
*/

uniform sampler2D uBackground;

uniform ivec2 uScreenSize;

uniform float uTime;

uniform sampler2D uNoise;

uniform float uStaticNoiseStrength;
uniform float uStaticNoiseGrainSize;

in vec2 fUV;

out vec4 fragment;

void main()
{
	ivec2 xy = ivec2(vec2(uScreenSize) * fUV);
	xy /= int(uStaticNoiseGrainSize);

	ivec2 si = textureSize(uNoise, 0);

	xy += ivec2(uTime * (5 + texelFetch(uNoise, xy % si, 0).rg));

	fragment = texture(uBackground, fUV);

	fragment.rgb += uStaticNoiseStrength
		* (2.0 * texelFetch(uNoise, xy % si, 0).rgb - 1.0)
		;
}
