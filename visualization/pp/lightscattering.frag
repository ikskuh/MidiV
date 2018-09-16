#version 330

const int NUM_SAMPLES = 100 ;

/*
{
	"sources": [ "lightscattering.frag" ],
	"bindings": [
		{ "uniform": "uScatterWeight",   "value": 0.1 },
		{ "uniform": "uScatterDecay",    "value": 0.95 },
		{ "uniform": "uScatterDensity",  "value": 0.9 },
		{ "uniform": "uScatterExposure", "value": 1.0 },
		{ "uniform": "uScatterCenterX",  "value": 0.0 },
		{ "uniform": "uScatterCenterY",  "value": 0.4 }
	]
}
*/


uniform sampler2D uStage;
uniform sampler2D uStageData;

in vec2 fUV;
out vec4 fragment;

uniform float uScatterExposure;
uniform float uScatterDecay;
uniform float uScatterDensity;
uniform float uScatterWeight;
uniform float uScatterCenterX, uScatterCenterY;
uniform ivec2 uScreenSize;

void main()
{
	float aspect = float(uScreenSize.x) / float(uScreenSize.y);
	vec2 xy = vec2(uScatterCenterX / aspect, uScatterCenterY);
	xy *= 0.5;
	xy += 0.5;

   vec2 deltaTextCoord = (vec2(fUV - xy) / float(NUM_SAMPLES)) * uScatterDensity;
   vec2 uv = fUV;

   fragment = texture(uStage, fUV);

   float illuminationDecay = 1.0;
   for(int i = 0; i < NUM_SAMPLES ; i++)
   {
		uv -= deltaTextCoord;

		vec4 occlusion = texture(uStageData, uv );

		occlusion *= illuminationDecay * uScatterWeight;

		fragment += occlusion;

		illuminationDecay *= uScatterDecay;
	}

	fragment *= uScatterExposure;
}
