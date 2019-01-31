#version 330
// From vertex shader
in vec2 texcoord;
in vec2 vpos;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float lightRadius;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = texture(sampler0, vec2(texcoord.x, texcoord.y));
}
