#version 330
// From vertex shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform vec3 fcolor;
uniform float fireflyRadius;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = vec4(fcolor * vcolor, 1.0);
	
	float radius = distance(vec2(0.0), vpos);
	if (radius < fireflyRadius)
	{
		color.xyz = vec3(1.0, 1.0, 1.0);
		color.a = 1;
	}
	else
	{
		color.a = 0;
	}
}
