#version 330

/*
{
	"sources": [ "analog-glitch.frag" ],
	"bindings": [
		{ "uniform": "uGlitchStrength", "note": 64, "scale": 0.25 },
		{ "uniform": "uStaticGlitchStrength", "value": 24.0 }
	]
}
*/

uniform sampler2D uStage;

uniform ivec2 uScreenSize;

uniform float uGlitchStrength;
uniform float uStaticGlitchStrength;

uniform float uTime;

uniform sampler2D uNoise;

in vec2 fUV;

out vec4 fragment;

void main()
{
	vec2 xy = vec2(uScreenSize) * fUV;

	float f = 0.3 * xy.y;
	float g = f + 32.0 * uTime;
	float h = f + 64.0 * uTime;
	vec2 uv = fUV;

	// Dynamic noise
	uv.x += 1.0
	     * uGlitchStrength
		 * pow(texture(uNoise, vec2(floor(g) / 64.0, 0)).x, 3.0)
		 * sin(3.14152 * g)
	     ;

	// Static noise
	uv.x += (uStaticGlitchStrength / float(uScreenSize.x))
		 * pow(texture(uNoise, vec2(floor(h) / 64.0, 0)).x, 3.0)
		 * sin(3.14152 * h)
	     ;

	fragment = mix(
		texture(uStage, uv),
		texture(uStage, fUV),
		0.5);
}
