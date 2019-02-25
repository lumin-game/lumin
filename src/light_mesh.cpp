  // Header
#include "light_mesh.hpp"
#include "CollisionManager.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

// external
#include "world.hpp"

bool LightMesh::init()
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

	CollisionManager::GetInstance().RegisterLight(this);

	return true;
}

// Releases all graphics resources
void LightMesh::destroy()
{
	CollisionManager::GetInstance().UnregisterLight(this);

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
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
	// TODO: if needed, we could further try and optimize by combining adjacent walls into one equation...
	ParametricLines collisionEquations = CollisionManager::GetInstance().CalculateLightEquations(m_parent.m_position.x, m_parent.m_position.y, m_lightRadius);

	// Send our list of collision equations as a vec4.
	if (collisionEquations.size() > 0)
	{
		glUniform4fv(collision_eqs, 4 * collisionEquations.size(), (float*)&collisionEquations[0]);
		glUniform1i(collision_eqs_count, collisionEquations.size());
	}

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 LightMesh::get_position() const
{
	return m_parent.m_position;
}

float LightMesh::getLightRadius() const
{
	return m_lightRadius;
}