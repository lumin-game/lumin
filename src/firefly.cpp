#include "firefly.hpp"
#include <random>

bool Firefly::init()
{
	// Setting initial values, scale is negative to make it face the opposite way
// 1.0 would be as big as the original texture
	m_scale.x = 1.f;
	m_scale.y = 1.f;

	m_position = { 0.f, 50.f };
	m_screen_pos = m_position;

	std::random_device rand;
	std::mt19937 gen(rand());
	std::uniform_real_distribution<> dis(-FIREFLY_DISTRIBUTION, FIREFLY_DISTRIBUTION);
	for (int i = 0; i < FIREFLY_COUNT; ++i)
	{
		fireflies.push_back(SingleFirefly(dis(gen), dis(gen)));
	}

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	Vertex vertex;
	vertex.color = { 1.f, 1.f, 1.f };

	//for (SingleFirefly firefly : fireflies)
	//{
	SingleFirefly firefly = fireflies[0];
		int vertexCount = vertices.size();
		float right = m_position.x + firefly.position.x + FIREFLY_RADIUS;
		float left = m_position.x + firefly.position.x - FIREFLY_RADIUS;
		float top = m_position.y + firefly.position.y + FIREFLY_RADIUS;
		float bottom = m_position.y + firefly.position.y - FIREFLY_RADIUS;

		vertex.position = { right, top, -0.02f };
		vertices.push_back(vertex);
		vertex.position = { right, bottom, -0.02f };
		vertices.push_back(vertex);
		vertex.position = { left, bottom, -0.02f };
		vertices.push_back(vertex);
		vertex.position = { left, top, -0.02f };
		vertices.push_back(vertex);

		indices.push_back(1 + vertexCount);
		indices.push_back(3 + vertexCount);
		indices.push_back(2 + vertexCount);
		indices.push_back(0 + vertexCount);
		indices.push_back(3 + vertexCount);
		indices.push_back(1 + vertexCount);
// 	}

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices.data(), GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("firefly.vs.glsl"), shader_path("firefly.fs.glsl")))
		return false;

	if (!lightMesh.init())
	{
		return false;
	}

	return true;
}

void Firefly::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()

	transform_translate(m_screen_pos);
	transform_scale(m_scale);

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

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Firefly::get_screen_pos() const {
	return m_screen_pos;
}

vec2 Firefly::get_position() const {
	return m_position;
}

void Firefly::set_screen_pos(vec2 position) {
	m_screen_pos = position;
}
