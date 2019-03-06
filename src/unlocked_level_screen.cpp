#include "unlocked_level_screen.hpp"

#include <iostream>
#include <cmath>

Texture UnlockedLevelScreen::unlocked_level_screen_texture;

bool UnlockedLevelScreen::init() {

	// Since we are not going to apply transformation to this screen geometry
	// The coordinates are set to fill the standard openGL window [-1, -1 .. 1, 1]
	// Make the size slightly larger then the screen to crop the boundary
  if (!unlocked_level_screen_texture.is_valid())
  {
    if (!unlocked_level_screen_texture.load_from_file(textures_path("level_unlocked.png")))
    {
      fprintf(stderr, "Failed to load level screen texture!");
      return false;
    }
  }

  // The position corresponds to the center of the texture
  // TODO: ensure that texture size is the same
  float wr = unlocked_level_screen_texture.width * 0.5f;
  float hr = unlocked_level_screen_texture.height * 0.5f;
  std::cout << wr << " " << hr << std::endl;

  vertices[0].position = { -wr, +hr, 0.f };
	vertices[1].position = { +wr, +hr, 0.f };
	vertices[2].position = { +wr, -hr, 0.f };
	vertices[3].position = { -wr, -hr, 0.f };

	vertices[0].texcoord = { 0.f, 1.f };//top left
	vertices[1].texcoord = { 1.f, 1.f };//top right
	vertices[2].texcoord = { 1.f, 0.f };//bottom right
	vertices[3].texcoord = { 0.f, 0.f };//bottom left

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
	m_scale = { 1.0, 1.0 };
	// m_position = position; // modify this later

  return true;
}


void UnlockedLevelScreen::destroy() {
  glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void UnlockedLevelScreen::draw(const mat3& projection) {
  transform_begin();
  transform_scale(m_scale);
  transform_translate(m_position);
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
  glBindTexture(GL_TEXTURE_2D, unlocked_level_screen_texture.id);

  // Setting uniform values to the currently bound program
  glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
  float color[4] = { 1.f, 1.f, 1.f, 1.f };
  glUniform4fv(color_uloc, 1, color);
  glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

  // Drawing!
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void UnlockedLevelScreen::set_screen_position(vec2 position) {
	m_position = position;
}

// returns the local bounding coordinates scaled by the current size of the background
vec2 UnlockedLevelScreen::get_bounding_box()const
{
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * unlocked_level_screen_texture.width, std::fabs(m_scale.y) * unlocked_level_screen_texture.height };
}

