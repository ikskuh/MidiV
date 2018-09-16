#version 330

const vec4 fogColor = vec4(0.2, 0.2, 0.3, 1.0);

/*
{
	"sources": [ "tunnel.frag" ],
	"bindings": [
		{
			"uniform": "uTunnelPosition",
			"note": 38,
			"channel": 9,
			"scale": 1.5,
			"integrate": true
		},
		{
			"uniform": "uTunnelCenterX",
			"source": "pitch"
		},
		{
			"uniform": "uTunnelCenterY",
			"value": -0.5
		},
		{
			"uniform": "uTunnelRadiusX",
			"value": 0.3
		},
		{
			"uniform": "uTunnelRadiusY",
			"value": 0.3
		},
		{
			"uniform": "uTunnelProgressX",
			"value": 0.5,
			"integrate": true
		},
		{
			"uniform": "uTunnelProgressY",
			"value": 0.5,
			"integrate": true
		}
	]
}
*/

uniform float uTunnelPosition;

in vec2 fUV;
out vec4 fragment;

uniform float uTime;
uniform ivec2 uScreenSize;

////////////////////////////////////////////////////////////////////////////////

uniform sampler2D uStage;

uniform float uTunnelCenterX, uTunnelCenterY;
uniform float uTunnelRadiusX, uTunnelRadiusY;
uniform float uTunnelProgressX, uTunnelProgressY;

void main()
{
	vec2 xy = 2.0 * fUV - 1.0;
	xy.x *= float(uScreenSize.x) / float(uScreenSize.y);

	xy.x -= uTunnelCenterX;
	xy.y -= uTunnelCenterY;

	xy.x += uTunnelRadiusX * sin(uTunnelProgressX);
	xy.y += uTunnelRadiusY * cos(uTunnelProgressY);

	vec2 uv = vec2(
		4.0 * atan(xy.y, xy.x) / 6.28,
		1.0 / length(xy) + uTunnelPosition
		);

	fragment = mix(
		texture(uStage, uv),
		fogColor,
		clamp(pow(0.15 / length(xy), 0.75) - 0.4, 0.0, 1.0));
}
