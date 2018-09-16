#version 330

// Uniform pixel coordinates in range [0;1]
in vec2 fUV;

// Output color
out vec4 fragment;

// Size of the screen in pixels
uniform ivec2 uScreenSize;

// Contains a 1D texture per channel with all note values in `.x` and all CC values in `.y`.
// `z` contains the note values integrated over time,
// `w` contains the CC values over time.
uniform sampler1DArray uChannels;

uniform sampler2D rNoiseRGBA;

void main()
{
	vec2 xy = vec2(128.0, 15.0) * fUV;
	ivec2 item = ivec2(xy);

	// filter out the drum patterns
	if(item.y >= 9)
		item.y += 1;

	vec2 uv = fract(xy);
	xy = 2.0 * uv - 1.0;

    vec2 border = 5.0 * max(vec2(0), abs(xy) - 0.8);

	float p = texelFetch(uChannels, item, 0).x;

	vec3 col = texelFetch(rNoiseRGBA, ivec2(vec2(2.8, 3.9) * vec2(item)) % ivec2(128,128), 0).rgb;

	if(length(col) < 0.7)
		col = 0.7 * normalize(col);

	fragment.rgb = (1.0 - length(border)) * (0.8 * p * col + 0.2);
    fragment.a = 1.0;
}
