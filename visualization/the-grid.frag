#version 330

// Plasmose is meant as a reference shader impementation in Midi-V containing
// all available uniforms and input values

// Uniform pixel coordinates in range [0;1]
in vec2 fUV;

// Output color
out vec4 fragment;

////////////////////////////////////////////////////////////////////////////////

// Time since visualization start
uniform float uTime;

// Time since the last frame
uniform float uDeltaTime;

// Size of the screen in pixels
uniform ivec2 uScreenSize;

// Contains a 1D texture per channel with all note values in `.x` and all CC values in `.y`.
// `z` contains the note values integrated over time,
// `w` contains the CC values over time.
uniform sampler1DArray uChannels;

// Contains the sum of all channels (note,cc) note values except channel 9 (drums)
// `x` contains the current value, `y` contains the integrated value.
uniform sampler1D uFFT;

// Contains the previously rendered image in the shader chain or the output
// of the last shader from the previous frame.
uniform sampler2D uBackground;

////////////////////////////////////////////////////////////////////////////////

const float horizon = 0.4;
const float skyline = 0.7;

uniform sampler2D rGridTexture;
uniform sampler2D rGridSky;

const vec4 fogColor = vec4(0.2, 0.2, 0.3, 1.0);

void main()
{
	float sy = 1.0 - fUV.y;
	if(sy >= horizon)
	{
		float y = (sy - horizon) / (1.0 - horizon);

		float x = 2.0 * fUV.x - 1.0;
		x *= float(uScreenSize.x) / float(uScreenSize.y);

		vec2 uv = vec2(4 * x / y, 4.0 / y);

		uv.y += 2.0 * uTime;

		fragment = mix(
			textureLod(rGridTexture, uv, 5.5 * (1.0 - y)),
			fogColor,
			pow(1.0 - y, 7.0));
	}
	else
	{
		float h = skyline * textureLod(uFFT, fUV.x, 2).x;

		float thickness = 2.0 / float(uScreenSize.y);

		float y0 = (horizon - sy) / (1.0 - horizon);
		float y1 = (horizon - sy + thickness) / (1.0 - horizon);

		const vec3 skylineColor = vec3(0.176, 0.35, 0.93);

		if(h < y0 && h < y1)
		{
			if(y0 <= thickness)
			{
				fragment = vec4(skylineColor, 1.0);
			}
			else
			{
				fragment = texelFetch(rGridSky, ivec2(gl_FragCoord.xy) % ivec2(textureSize(rGridSky, 0)), 0 );
			}
		}
		else if(h >= y0 && h < y1)
		{
			fragment = vec4(skylineColor, 1.0);
		}
		else
		{
			fragment = fogColor;
		}
	}
}
