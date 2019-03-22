#version 330
// From vertex shader
in vec3 vcolor;
in vec2 vpos;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float lightWidth;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1,1,1,1);
	color.a = (lightWidth - abs(vpos.x))/lightWidth;
}
