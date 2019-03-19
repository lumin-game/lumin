#version 330 
// Input attributes
in vec3 in_position;
in vec3 in_color;

// Passed to fragment shader
out vec2 vpos;
out vec3 vcolor;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main()
{
	vpos = in_position.xy;
	vcolor = in_color;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.x, pos.y, in_position.z, 1.0);
}