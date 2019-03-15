#version 330
// From vertex shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float lightRadius;
uniform int showPolygon;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	if (showPolygon == 1)
	{
		color = vec4(1,1,1,1);
		return;
	}
	//TODO: Prolly don't hardcode these values
	float radius = distance(vec2(0.0), vpos);
	if (radius < lightRadius)
	{
		color.xyz = vec3(1.0, 1.0, 1.0);
		color.a = max(0.00, min(0.8, (1 - radius / lightRadius)));
	}
	else
	{
		color.a = 0;
	}
}
