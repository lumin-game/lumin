#version 330
uniform sampler2D screen_texture;
in vec2 uv;
uniform float dead_timer;
uniform bool should_darken;

layout(location = 0) out vec4 color;

vec4 fade_color(vec4 in_color) {
	vec4 color = in_color;
	if (should_darken)
	{
		color += dead_timer * vec4(0.1, 0.1, 0.1, 0);
	}
	else
	{
	    color -= dead_timer * vec4(0.1, 0.1, 0.1, 0);
	}

	return color;
}

void main()
{
    color = texture(screen_texture, uv);
    color = fade_color(color);
}