#version 330

uniform float uPitchWheel;

uniform float uGrayLevel;

uniform sampler2D uBackground;

out vec4 fragment;

in vec2 fUV;

float average(vec3 rgb)
{
	return (rgb.r + rgb.g + rgb.b) / 3.0;
}

float lightness(vec3 rgb)
{
	return (max(max(rgb.r, rgb.g), rgb.b) + min(min(rgb.r, rgb.g), rgb.b)) / 2.0;
}

float luminosity(vec3 rgb)
{
	return 0.21 * rgb.r + 0.72 * rgb.g + 0.07 * rgb.b;
}

void main()
{
	vec4 color = texture(uBackground, fUV);

	float f = abs(uPitchWheel) - min(0.0, uPitchWheel);

	fragment = mix(
		vec4(vec3(luminosity(color.rgb)), color.a),
		color,
		uGrayLevel + f);
}
