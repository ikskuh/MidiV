#version 330

uniform float uTime;
uniform float uDeltaTime;

uniform float uVolume;

uniform sampler2D uBackground;
uniform sampler2D rPattern;

uniform sampler1DArray uChannels;

uniform ivec2 uScreenSize;

in vec2 fUV;

out vec4 fragment;

void main()
{
	float i = 4.0 * textureLod(uChannels, vec2(0.3, 9.0), 4.0).x;

	vec4 now = vec4(0.0);
	vec4 then = texture(uBackground, fUV - vec2(0.3 * i * uDeltaTime, 0.0));

	fragment = mix(then, now, 0.0);
	if(fUV.x < 0.05)
	{
		float p = 0.5 + 0.3 * sin(uTime);

		fragment = vec4(pow(max(0.0, 1.0 - 5.0 * abs(fUV.y - p)), 8.0), 0.0, 0.0, 1.0);
	}
}
