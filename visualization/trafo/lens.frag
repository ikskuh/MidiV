#version 330

/*
{
	"sources": [ "trafo/lens.frag" ],
	"bindings": [
		{ "uniform": "uLensRadius",    "value": 0.5 },
		{ "uniform": "uLensCurvature", "value": 2.0 },
		{ "uniform": "uLensCenterX",   "value": 0.0 },
		{ "uniform": "uLensCenterY",   "value": 0.5 }
	]
}
*/

uniform ivec2 uScreenSize;

uniform sampler2D uStage;

in vec2 fUV;

out vec4 fragment;

uniform float uLensRadius = 0.5;
uniform float uLensCurvature = 2.0;

uniform float uLensCenterX = 0.0;
uniform float uLensCenterY = 0.5;

void main()
{
	float aspect = float(uScreenSize.x) / float(uScreenSize.y);

	vec2 xy = 2.0 * fUV - 1.0;
	xy -= vec2(uLensCenterX, uLensCenterY);
	xy.x *= aspect;

	vec2 dist = xy / uLensRadius;

	float d = length(dist);
	if(d <= 1.0)
	{
		xy = uLensRadius * normalize(xy) * pow(d, uLensCurvature);
	}

	xy.x /= aspect;
	xy += vec2(uLensCenterX, uLensCenterY);

	vec2 uv = 0.5 + 0.5 * xy;

	fragment = texture(uStage, uv);
}
