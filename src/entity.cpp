#include "entity.hpp"

#include <cmath>
#include <iostream>
#include "CollisionManager.hpp"


bool Entity::init(float x_pos, float y_pos) {
	if (!unlit_texture.load_from_file(get_texture_path())) {
		fprintf(stderr, "Failed to load entity texture!");
		return false;
	}

	if (get_lit_texture_path() != nullptr && !lit_texture.load_from_file(get_lit_texture_path())) {
		fprintf(stderr, "Failed to load lit entity texture!");
		return false;
	}

	texture = m_is_lit ? &lit_texture : &unlit_texture;

	// The position corresponds to the center of the texture
	float wr = texture->width * 0.5f;
	float hr = texture->height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.02f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.02f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, -0.02f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.02f };
	vertices[3].texcoord = { 0.f, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 0.5f;
	m_scale.y = 0.5f;

	m_position.x = (float)x_pos;
	m_position.y = (float)y_pos;

	CollisionManager::GetInstance().RegisterEntity(this);

	m_entity_sound = Mix_LoadWAV(get_audio_path());

	if (get_audio_path() != nullptr && m_entity_sound == nullptr) {
		fprintf(stderr, "Failed to load audio file!");
		return false;
	}

	return true;
}

// Call if init() was successful
// Releases all graphics resources
void Entity::destroy() {
	CollisionManager::GetInstance().UnregisterEntity(this);

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	if (m_entity_sound != nullptr)
		Mix_FreeChunk(m_entity_sound);

	effect.release();
}

void Entity::UpdateHitByLight()
{
	if (activated_by_light())
	{
		if (m_is_lit && !m_was_lit) {
			activate();
		}
		else if (!m_is_lit && m_was_lit) {
			deactivate();
		}
	}

	m_was_lit = m_is_lit;
	m_is_lit = false;


}

void Entity::update(float elapsed_ms) {
}

void Entity::draw(const mat3& projection) {
	if (!alwaysRender() && !get_lit())
	{
		return;
	}

	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform_begin();
	transform_translate(m_position);
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

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	EntityColor color = get_color();
	float fvColor[4] = { color.r, color.g, color.b, color.a };
	glUniform4fv(color_uloc, 1, fvColor);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Entity::get_position() const {
	return m_position;
}

void Entity::set_position(vec2 position) {
	m_position = position;
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 Entity::get_bounding_box() const {
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * texture->width, std::fabs(m_scale.y) * texture->height };
}

ParametricLines Entity::calculate_static_equations() const {
	if (!is_light_collidable()) {
		return 	ParametricLines();
	}
	return calculate_boundary_equations();
}

void Entity::set_lit(bool lit) {
	m_is_lit = lit;
	texture = lit ? &lit_texture : &unlit_texture;
}

bool Entity::get_lit() const {
	return m_is_lit;
}
ParametricLines Entity::calculate_dynamic_equations() const
{
	// By default entities have no dynamic equations
	return ParametricLines();
}

ParametricLines Entity::calculate_boundary_equations() const
{
	ParametricLines outLines;

	// Create 4 lines for each each of the box and returns them
	vec2 boundingBox = get_bounding_box();
	float xHalf = boundingBox.x / 2;
	float yHalf = boundingBox.y / 2;

	float rightBound = m_position.x + xHalf;
	float leftBound = m_position.x - xHalf;
	float topBound = m_position.y + yHalf;
	float bottomBound = m_position.y - yHalf;

	ParametricLine rightEdge;
	rightEdge.x_0 = rightBound;
	rightEdge.x_t = 0.f;
	rightEdge.y_0 = bottomBound;
	rightEdge.y_t = topBound - bottomBound;

	ParametricLine leftEdge;
	leftEdge.x_0 = leftBound;
	leftEdge.x_t = 0.f;
	leftEdge.y_0 = bottomBound;
	leftEdge.y_t = topBound - bottomBound;

	ParametricLine topEdge;
	topEdge.x_0 = leftBound;
	topEdge.x_t = rightBound - leftBound;
	topEdge.y_0 = topBound;
	topEdge.y_t = 0.f;

	ParametricLine bottomEdge;
	bottomEdge.x_0 = leftBound;
	bottomEdge.x_t = rightBound - leftBound;
	bottomEdge.y_0 = bottomBound;
	bottomEdge.y_t = 0.f;

	outLines.push_back(rightEdge);
	outLines.push_back(leftEdge);
	outLines.push_back(topEdge);
	outLines.push_back(bottomEdge);

	return outLines;
}


void Entity::register_entity(Entity* entity) {
	m_entities.insert(entity);
}

Mix_Chunk* Entity::get_sound() const{
	return m_entity_sound;
}

