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

uniform sampler1D rGradient;

uniform float uPlasmaScale;

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float plasma(vec2 uv, float iTime)
{
	uv *= 2.0;
	uv -= 1.0;

	uv *= uPlasmaScale;

	return sin(uv.x + iTime)
		+ 0.4 * sin(1.3 * uv.y + 0.3 * iTime)
		+       sin(1.0 * uv.y + iTime + 2)
		+ 1.2 * sin(0.3 * uv.y + 0.7 * iTime)
		;
}

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
	fragment = vec4(0)
		+ texture(rGradient, plasma(fUV, t))
		+ pow(textureLod(uFFT, 0.7, 4.0).xxxx, vec4(0.25))
		// + 4.0 * textureLod(uChannels, vec2(fUV.x, 15.0 * fUV.y), 4.0)
	    ;
}
