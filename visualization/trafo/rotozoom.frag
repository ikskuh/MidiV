#version 330

/*
{
	"sources": [ "trafo/rotozoom.frag" ],
	"bindings": [
		{ "uniform": "uZoom", "value": 0.8, "integrate": true },
		{ "uniform": "uRoto", "value": 0.1, "integrate": true },
		{ "uniform": "uMovX", "value": 1.0, "integrate": true },
		{ "uniform": "uMovY", "value": 2.0, "integrate": true },
		{ "uniform": "uZoomMin", "value": 0.75 },
		{ "uniform": "uZoomMax", "value": 3.5 }
	]
}
*/

in vec2 fUV;
out vec4 fragment;

uniform float uTime;
uniform sampler2D uStage;
uniform ivec2 uScreenSize;

uniform float uZoom;
uniform float uRoto;
uniform float uMovX;
uniform float uMovY;
uniform float uZoomMin, uZoomMax;

void main()
{
	float aspect = float(uScreenSize.x) / float(uScreenSize.y);

	vec2 uv = fUV;
	uv.x -= 0.5;
	uv.x *= aspect;

	// zoom
	float zoom = uZoomMin + (uZoomMax - uZoomMin) * (0.5 + 0.5 *sin(uZoom));
	uv *= zoom;

	// roto
	float a = uRoto;
	uv = vec2(
		uv.x * sin(a) - uv.y * cos(a),
		uv.x * cos(a) + uv.y * sin(a));

	// translate
	uv.x += uMovX;
	uv.y += uMovY;

	uv.x /= aspect;
	uv += 0.5;

	fragment = texture(uStage, uv);
}
