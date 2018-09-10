#version 330

/*
{
	"sources": [ "pixelize.frag" ],
	"bindings": [
		{ "uniform": "uPixelSize", "value": 8 }
	]
}
*/

uniform sampler2D uBackground;

uniform ivec2 uScreenSize;

uniform float uPixelSize;

uniform sampler2D uNoise;

in vec2 fUV;

out vec4 fragment;

const int numSamples = 3;

void main()
{
	ivec2 xy = ivec2(vec2(uScreenSize) * fUV);
	xy /= int(uPixelSize);
	xy *= int(uPixelSize);

	fragment = vec4(0);
	for(int x = 0; x < numSamples; x++)
	{
		for(int y = 0; y < numSamples; y++)
		{
			vec2 uv = (vec2(xy) + (uPixelSize - 1.0) * vec2(x,y) / (numSamples - 1)) / vec2(uScreenSize);

			fragment += texture(uBackground, uv);
		}
	}
	fragment /= vec4(float(numSamples * numSamples));
}
