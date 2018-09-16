#version 330

#define REDUCE_SAMPLING_ARTIFACTS

const vec4 fogColor = vec4(1.0, 0.42, 0.94, 1.0); // vec4(0.2, 0.2, 0.3, 1.0);
const vec4 gridColor = vec4(1.0, 0.0, 0.9, 1.0);
const vec4 skylineColor = vec4(0.176, 0.35, 0.93, 1.0);

const vec2 sunCenter = vec2(0.0, 0.4);
const float sunRadius = 0.6;

const float horizon = 0.4;
const float skyline = 0.7;

/*
{
	"sources": [ "the-grid.frag" ],
	"bindings": [ ]
}
*/

// Uniform pixel coordinates in range [0;1]
in vec2 fUV;

// Output color
layout(location = 0) out vec4 fragment;
layout(location = 1) out vec4 fragmentData;

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

uniform sampler2D rGridTexture;
uniform sampler2D rGridSky;
uniform sampler2D rGridSun;

uniform sampler2D uNoise;

vec4 renderField(vec2 uv_in)
{
	float sy = 1.0 - uv_in.y;
	float y = (sy - horizon) / (1.0 - horizon);

	float x = 2.0 * uv_in.x - 1.0;
	x *= float(uScreenSize.x) / float(uScreenSize.y);

	vec2 uv = vec2(4 * x / y, 4.0 / y);

	uv.y += 2.0 * uTime;

	ivec2 ab = ivec2(uv);

	vec4 rng = texture(uNoise, vec2(0.14, 0.32) * ab);

	uv = fract(uv);

	vec2 xy = 2.0 * uv - 1.0;

	float l = 0.0;
	for(int i = ab.x % 8; i < 128; i += 8)
	{
		for(int j = ab.y % 8; j < 16; j += 8)
		{
			l += texture(uChannels, vec2(float(i) / 127.0 + rng.x, mod(j + 15.0 * rng.y, 16))).x;
		}
	}

	/*
	vec3 texel = mix(
		vec3(l,l,l),
		vec3(0,0,0),
		pow(clamp(max(2.0 * abs(xy.x) - 1.0, 2.0 * abs(xy.y) - 1.0), 0.0, 1.0), 5.0));
	*/
	vec3 texel = textureLod(rGridTexture, uv, 4.0 * (1.0 - y)).rgb;

	texel = mix(
		vec3(l) * normalize(rng.rgb),
		gridColor.rgb,
		max(texel.r, max(texel.g, texel.b)));

	return vec4(texel, 1.0);
}

void main()
{
	fragmentData = vec4(0,0,0,0);

	float sy = 1.0 - fUV.y;
	if(sy >= horizon)
	{
		float y = (sy - horizon) / (1.0 - horizon);
#ifdef REDUCE_SAMPLING_ARTIFACTS
		fragment = vec4(0);
		for(int x = -1; x <= 1; x++)
		{
			for(int y = -1; y <= 1; y++)
			{
				fragment += renderField(fUV + 0.5 * vec2(x,y) / vec2(uScreenSize) );
			}
		}
		fragment /= 9;
#else
		fragment = renderField(fUV);
#endif

		fragment = mix(
			fragment,
			fogColor,
			pow(1.0 - y, 4.0));
	}
	else
	{
		float h = skyline * textureLod(uFFT, fUV.x, 2).x;

		float thickness = 2.0 / float(uScreenSize.y);

		float y0 = (horizon - sy) / (1.0 - horizon);
		float y1 = (horizon - sy + thickness) / (1.0 - horizon);

		if(h < y0 && h < y1)
		{
			if(y0 <= thickness)
			{
				fragment = skylineColor;
			}
			else
			{
				float aspect = float(uScreenSize.x) / float(uScreenSize.y);

				vec2 xy = vec2(aspect, 1.0) * (2.0 * fUV - 1.0);

				vec4 sky = texelFetch(rGridSky, ivec2(gl_FragCoord.xy) % ivec2(textureSize(rGridSky, 0)), 0 );

				if(distance(sunCenter, xy) <= sunRadius)
				{
					float r = clamp(1.1 * (distance(sunCenter, xy) - sunRadius) / sunRadius, 0.0, 1.0);

					vec4 sun = texture(rGridSun, vec2(sunCenter - xy - sunRadius) / (2.0 * sunRadius));

					float b = max(sun.r, max(sun.g, sun.b));

					fragment = mix(sky, sun, b * r);

					fragmentData = sun * pow(b, 3.0);
				}
				else
				{
					fragment = sky;
				}
			}
		}
		else if(h >= y0 && h < y1)
		{
			fragment = skylineColor;
		}
		else
		{
			const float tessLevel = 20.0f;

			vec3 texel = textureLod(rGridTexture,
				vec2(
					tessLevel * y0 * pow(1.0 + h, 1.3),
					100 * fUV.x
				),
				2).rgb;

			fragment = mix(
				vec4(0,0,0,1),
				skylineColor,
				max(texel.r, max(texel.g, texel.b)));
		}
	}
}
