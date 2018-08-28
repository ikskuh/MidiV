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

// Mouse position in pixels
uniform ivec2 uMousePos;

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

uniform sampler1D rGradient;

void main()
{
	float aspect = float(uScreenSize.x) / float(uScreenSize.y);
	vec2 xy = vec2(aspect, 1.0) * fUV;
	float i = 4.0 * textureLod(uChannels, vec2(0.3, 9.0), 4.0).z;
	float t = uTime + 10.0 * i;
	float v = 0
		+ 0.6 * sin( 1.1 * t + 23.0 * xy.x)
		+ 0.5 * sin(-0.2 * t + 19.4 * xy.y)
		+ 0.4 * sin( 1.3 * t + 17.0 * xy.x)
		+ 0.3 * sin(-1.4 * t + 13.2 * xy.y)
		+ 0.2 * sin(-0.5 * t + 11.0 * xy.x)
		+ 0.1 * sin( 1.6 * t +  7.0 * xy.y)
		;

	float value = 0.5 + v / 4.2;
	fragment = texture(rGradient, value)
		+ pow(textureLod(uFFT, 0.7, 4.0).xxxx, vec4(0.25))
		+ 4.0 * textureLod(uChannels, vec2(fUV.x, 15.0 * fUV.y), 4.0);
}
