#version 330

in vec2 fUV;
out vec4 fragment;

uniform ivec2 uScreenSize;
uniform sampler2D uStage;
uniform float uTime;

struct Ball
{
	vec2 pos;
	float size;
	vec3 color;
};

void main()
{
	float aspect = float(uScreenSize.x) / float(uScreenSize.y);

	Ball[] balls = Ball[](
		Ball(0.7 * vec2(sin(uTime), cos(1.2 * uTime)),       1.5, vec3(1,0,0)),
		Ball(0.6 * vec2(sin(1.3 * uTime), cos(0.9 * uTime)), 0.5, vec3(0,1,0)),
		Ball(0.5 * vec2(sin(0.8 * uTime), cos(1.0 * uTime)), 1.0, vec3(0,0,1))
	);

	vec2 xy = 2.0 * fUV - 1.0;
	xy.x *= aspect;

	float density = 0.0;
	vec3 color = vec3(0.0);
	for(int i = 0; i < balls.length(); i++)
	{
		float d = 1.0 / distance(balls[i].pos, xy);
		density += d * balls[i].size;
		color += d * balls[i].color;
	}

	if(density >= 10.0)
	{
		// clamp(pow(0.2 * (density - 10.0), 0.5), 0.0, 1.0)
		vec3 c = color / density;
		fragment = vec4(c, 1.0);
	}
	else
	{
		fragment = texture(uStage, fUV);
	}
}
