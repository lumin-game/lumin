  // Header
#include "light_mesh.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

// external
#include "world.hpp"

bool LightMesh::init(const World* world)
{
	m_lightRadius = 300.f;

	// We set the mesh as a square with side length radius * 2.
	// Because circles are difficult....
	// Note that there is no texture here
	std::vector<Vertex> vertices;

	Vertex vertex;
	vertex.color = { 1.f, 1.f, 1.f };

	vertex.position = { m_lightRadius, m_lightRadius, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { m_lightRadius, -m_lightRadius, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { -m_lightRadius, -m_lightRadius, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { -m_lightRadius, m_lightRadius, -0.02f };
	vertices.push_back(vertex);

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = {1, 3, 2, 0, 3, 1};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("light.vs.glsl"), shader_path("light.fs.glsl")))
		return false;

	// Remember the collision equations for walls that don't move
	m_staticCollisionPtr = world->getStaticCollisionLines();

	return true;
}

// Releases all graphics resources
void LightMesh::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void LightMesh::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()

	transform_translate(m_parent.m_screen_pos);

	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint light_radius = glGetUniformLocation(effect.program, "lightRadius");
	GLint collision_eqs = glGetUniformLocation(effect.program, "collisionEqs");
	GLint collision_eqs_count = glGetUniformLocation(effect.program, "collisionEqCount");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_color_loc = glGetAttribLocation(effect.program, "in_color");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
	
	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	glUniform1f(light_radius, m_lightRadius);

	// Now we update our collision equations based on where we are in the world
	// TODO: optimize so that we only check these collisions if wall is close enough to player
	// TODO: if needed, we could further try and optimize by combining adjacent walls into one equation...
	m_collisionEquations.clear();
	if (m_staticCollisionPtr) // Make sure init() has already happened
	{
		for (const ParametricLine& staticLine : *m_staticCollisionPtr)
		{
			// Since only position is at play, (and no scaling)
			// We only have to do a simple translation
			ParametricLine collEq;
			collEq.x_0 = staticLine.x_0 - m_parent.m_position.x;
			collEq.x_t = staticLine.x_t;
			collEq.y_0 = staticLine.y_0 - m_parent.m_position.y;
			collEq.y_t = staticLine.y_t;

			m_collisionEquations.push_back(collEq);
		}
		
		// Send our list of collision equations as a vec4.
		glUniform4fv(collision_eqs, 4 * m_collisionEquations.size(), (float*)&m_collisionEquations[0]);
		glUniform1i(collision_eqs_count, m_collisionEquations.size());

		// Note that we do not push anything to openGL if there are no equations.
		// Maybe that's a bad idea? Doesn't seem to cause a problem right now, but could fix later.
	}

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}