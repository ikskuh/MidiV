#version 330

/*
{
	"sources": [ "overlay/rasterbar.frag" ],
	"bindings": [
		{ "uniform": "uRasterPos",  "value": 0.5 },
		{ "uniform": "uRasterSize", "value": 0.2 }
	]
}
*/

in vec2 fUV;

out vec4 fragment;

uniform sampler2D uStage;

uniform float uRasterPos = 0.5;
uniform float uRasterSize = 0.2;

uniform sampler1D rRasterTexture;

void main()
{
	fragment = texture(uStage, fUV);

	float p = (fUV.y - uRasterPos) / (0.5 * uRasterSize);
	if(abs(p) <= 1.0)
	{
		fragment = texture(rRasterTexture, 0.5 + 0.5 * p);
	}
}
