#version 330

uniform float uTotalRotation;
uniform float uRotateSpeed;

uniform float uTime;

uniform ivec2 uScreenSize;

uniform sampler2D uStage;

in vec2 fUV;

out vec4 fragment;

void main()
{
	vec2 xy = 2.0 * fUV - 1.0;
	xy.x *= float(uScreenSize.x) / float(uScreenSize.y);

	float a = uRotateSpeed * uTime + uTotalRotation;

	vec2 r_xy = vec2(
		xy.x * cos(a) - xy.y * sin(a),
		xy.x * sin(a) + xy.y * cos(a));

	r_xy.x /= float(uScreenSize.x) / float(uScreenSize.y);

	vec2 uv = 0.5 + 0.5 * r_xy;

	fragment = texture(uStage, uv);
}
