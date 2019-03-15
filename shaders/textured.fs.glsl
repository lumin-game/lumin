#version 330
// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;
uniform float lightRadius;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
