#version 330
// From vertex shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform vec3 fcolor;
uniform float lightRadius;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = vec4(fcolor * vcolor, 1.0);

	if (true)
	{
		// 0.6 is just to make it not too strong
		color.xyz += 0.6 * vec3(1.0, 1.0, 1.0);
	}

	color = vec4(1.0, 1.0, 1.0, 1.0);

}
