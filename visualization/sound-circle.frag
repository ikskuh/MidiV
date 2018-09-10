#version 330
/*
{
	"sources": [ "sound-circle.frag" ],
	"bindings": [
		{ "uniform": "uRotateSpeed", "value": 1.0 },
		{ "uniform": "uCircleMinNote", "value": 0.3 },
		{ "uniform": "uCircleMaxNote", "value": 0.6 }
	]
}
*/

uniform ivec2 uScreenSize;

uniform sampler2D uBackground;

uniform sampler1D uFFT;


uniform float uCircleMinNote = 0.0;
uniform float uCircleMaxNote = 1.0;

in vec2 fUV;

out vec4 fragment;

const float PI = 3.14159265359;

float deg2rad(float f)
{
	return (f * PI) / 180.0;
}

void main()
{
	vec2 xy = 2.0 * fUV - 1.0;
	xy.x *= float(uScreenSize.x) / float(uScreenSize.y);

	float l = 1.5 * length(xy) - 1.0;

	float a = mod(atan(xy.y, xy.x) + deg2rad(180), 6.28) / 6.28;

	if(l <= 0.0)
	{
		fragment = vec4(0, 0, 0, 1);
	}
	else
	{
		l *= 3.0; // map to [0..1]

		float f = a;

		f *= (uCircleMaxNote - uCircleMinNote);
		f += uCircleMinNote;

		a = clamp(a, 0.0, 1.0);

		a *= 2.0;
		a -= 0.5;

		float n = textureLod(uFFT, f, 1).x;

		n = pow(n, 2.0);

		n *= pow(1.0 - abs(abs(a) - 1.0), 3.0);

		// fragment = vec4(n);
		// return;

// 		n *= 1.0 - abs(abs(a) - 1.0);

		if(n < l)
			fragment = texture(uBackground, fUV);
		else
			fragment.rgb = vec3(1,0,0);
	}
}
