#include "current_level.hpp"

#include <iostream>
#include <cmath>

Texture CurrentLevel::current_level_texture;

bool CurrentLevel::init(vec2 screen) {
  if (!current_level_texture.is_valid()) {
    if (!current_level_texture.load_from_file(textures_path("current_level.png")))
    {
      fprintf(stderr, "Failed to load level screen texture!");
      return false;
    }
  }

	float wr = 180 * 0.5f;
	float hr = current_level_texture.height * 0.5f;

	vertices[0].position = { -wr, +hr, 0.f };
	vertices[1].position = { +wr, +hr, 0.f };
	vertices[2].position = { +wr, -hr, 0.f };
	vertices[3].position = { -wr, -hr, 0.f };

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);

  m_current_level = 1;
  set_current_level_texture(m_current_level);
  // Vertex Array (Container for Vertex + Index buffer)
  glGenVertexArrays(1, &mesh.vao);
  if (gl_has_errors())
    return false;

  // Loading shaders
  if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
    return false;
  m_scale = { 0.1, 0.1 };
	m_position = { -460, -370 };
  return true;
}

void CurrentLevel::destroy() {
  glDeleteBuffers(1, &mesh.vbo);
  glDeleteBuffers(1, &mesh.ibo);
  glDeleteVertexArrays(1, &mesh.vao);
  effect.release();
}

int CurrentLevel::get_current_level() {
  return m_current_level;
}

void CurrentLevel::set_current_level_texture(int current_level) {
  m_current_level = current_level - 1; // starts at 0 index
  float tex_left = (float) m_current_level / TOTAL_LEVELS;
  float tex_right = (m_current_level + 1.f) / TOTAL_LEVELS;

  vertices[0].texcoord = { tex_left, 1.f };
  vertices[1].texcoord = { tex_right, 1.f };
  vertices[2].texcoord = { tex_right, 0.f };
  vertices[3].texcoord = { tex_left, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_DYNAMIC_DRAW);

	// Index Buffer creation
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_DYNAMIC_DRAW);
}

void CurrentLevel::update(int current_level) {
	set_current_level_texture(current_level);
}

void CurrentLevel::draw(const mat3& projection) {
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
  glBindTexture(GL_TEXTURE_2D, current_level_texture.id);

  // Setting uniform values to the currently bound program
  glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
  float color[4] = { 1.f, 1.f, 1.f, 1.f };
  glUniform4fv(color_uloc, 1, color);
  glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

  // Drawing!
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void CurrentLevel::set_position(vec2 position, vec2 offset) {
  m_position.x = position.x + offset.x;
  m_position.y = position.y + offset.y;
}

// returns the local bounding coordinates scaled by the current size of the background
vec2 CurrentLevel::get_bounding_box()const {
  // fabs is to avoid negative scale due to the facing direction
  return { std::fabs(m_scale.x) * current_level_texture.width, std::fabs(m_scale.y) * current_level_texture.height };
}
