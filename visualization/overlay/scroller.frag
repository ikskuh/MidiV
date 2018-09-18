#version 330

/*
{
	"sources": [ "overlay/scroller.frag" ],
	"bindings": [
		{ "uniform": "uScrollerPos",  "value": 0.5 },
		{ "uniform": "uScrollerSize", "value": 0.1 },
		{ "uniform": "uScrollerAmp", "value": 0.05 },
		{ "uniform": "uScrollerFreq", "value": 5.0 },
		{ "uniform": "uScrollerOff", "value": 5.0, "integrate": true },
		{ "uniform": "uScrollerScroll", "value": 0.5, "integrate": true }
	],
	"resources": [
		{
			"name": "rScrollerContent",
			"type": "text",
			"text": "Hello you, welcome to our demo!",
			"size": 24
		}
	]
}
*/

in vec2 fUV;

out vec4 fragment;

uniform sampler2D uStage;

uniform float uScrollerPos = 0.5;
uniform float uScrollerSize = 0.2;
uniform float uScrollerAmp = 0.0;
uniform float uScrollerOff = 0.0;
uniform float uScrollerFreq = 3.0;
uniform float uScrollerScroll;

uniform sampler2D rScrollerContent;

void main()
{
	fragment = texture(uStage, fUV);

	float dy = uScrollerAmp * sin(uScrollerOff + uScrollerFreq * fUV.x);

	float p = (fUV.y - uScrollerPos - dy) / (2.0 * uScrollerSize);
	if(abs(p) <= 1.0)
	{
		ivec2 size = textureSize(rScrollerContent, 0);
		float scale = 1.0 / (uScrollerSize * float(size.x) / float(size.y));
		float x = fUV.x * scale;
		float y = 0.5 - 0.5 * p;

		vec4 col = texture(rScrollerContent, vec2(uScrollerScroll * scale + x, y));
		fragment = mix(fragment, col, col.a);
	}
}
