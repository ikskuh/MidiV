#version 330

#define REDUCE_SAMPLING_ARTIFACTS

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

uniform sampler2D rTunnelSurface;

const vec4 fogColor = vec4(0.2, 0.2, 0.3, 1.0);

void main()
{
	vec2 xy = 2.0 * (fUV + vec2(0.3 * sin(uTime), 0.0)) - 1.0;
	xy.x *= float(uScreenSize.x) / float(uScreenSize.y);


	vec2 uv = vec2(
		4.0 * atan(xy.y, xy.x) / 6.28,
		1.0 / length(xy) + uTime
		);

	fragment = texture(rTunnelSurface, uv);
}
