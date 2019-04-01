#include "common.hpp"
#include "door.hpp"

bool Door::init(float x_pos, float y_pos) {
	Entity::init(x_pos, y_pos);
	m_position.y -= 20;
	return true;
}

bool Door::is_player_inside(Player* player) {
	return CollisionManager::GetInstance().BoxCollide(m_position, get_bounding_box(), player->get_position(), { 0.f, 0.f });
}

int Door::get_level_index() {
	return m_level_index;
}

void Door::set_level_index(int level) {
	m_level_index = level;
}

bool Door::is_enterable() {
	return is_open;
}

void Door::draw(const mat3& projection) {

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

	if (is_open) {
		glBindTexture(GL_TEXTURE_2D, lit_texture.id);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, unlit_texture.id);
	}

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	EntityColor color = get_color();
	float fvColor[4] = { color.r*darkness_modifier, color.g*darkness_modifier, color.b*darkness_modifier, color.a };

	glUniform4fv(color_uloc, 1, fvColor);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}
