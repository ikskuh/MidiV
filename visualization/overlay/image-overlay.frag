#version 330


uniform sampler2D uStage;

uniform sampler2D rImageOverlay;

in vec2 fUV;
out vec4 fragment;

void main()
{
	vec4 overlay = texture(rImageOverlay, fUV);
	vec4 color = texture(uStage, fUV);

	fragment = mix(
		color,
		overlay,
		overlay.a);
}
