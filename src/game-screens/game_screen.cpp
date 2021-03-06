#include "game_screen.hpp"

#include <iostream>
#include <cmath>

bool GameScreen::init(vec2 screen) {

	// Since we are not going to apply transformation to this screen geometry
	// The coordinates are set to fill the standard openGL window [-1, -1 .. 1, 1]
	// Make the size slightly larger then the screen to crop the boundary
    if (!screen_texture.load_from_file(get_texture_path()))
    {
      fprintf(stderr, "Failed to load game screen texture!");
      return false;
    }

  // The position corresponds to the center of the texture
  float wr = screen_texture.width * 0.5f;
  float hr = screen_texture.height * 0.5f;

  TexturedVertex vertices[4];
  vertices[0].position = { -wr, +hr, 0.f };
	vertices[0].texcoord = { 0.f, 1.f };//top left
	vertices[1].position = { +wr, +hr, 0.f };
	vertices[1].texcoord = { 1.f, 1.f };//top right
	vertices[2].position = { +wr, -hr, 0.f };
	vertices[2].texcoord = { 1.f, 0.f };//bottom right
	vertices[3].position = { -wr, -hr, 0.f };
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
  set_translation_scale();
  set_translation_position(screen, true);
  return true;
}

void GameScreen::destroy() {
  glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void GameScreen::draw(const mat3& projection) {
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
  glBindTexture(GL_TEXTURE_2D, screen_texture.id);

  // Setting uniform values to the currently bound program
  glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
  float color[4] = { 1.f, 1.f, 1.f, 1.f };
  glUniform4fv(color_uloc, 1, color);
  glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

  // Drawing!
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}
// returns the local bounding coordinates scaled by the current size of the background
vec2 GameScreen::get_bounding_box()const
{
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * screen_texture.width, std::fabs(m_scale.y) * screen_texture.height };
}

void GameScreen::set_position(vec2 position, vec2 offset) {
  m_position = position + offset;
}

void GameScreen::set_translation_scale() { 
  m_scale = { 1.f, 1.f }; 
};

void GameScreen::set_translation_position(vec2 screen, bool is_left) {
  m_position = calculate_position(screen, is_left);
};

vec2 GameScreen::calculate_position(vec2 screen, bool is_left) {
  if (m_scale.x == 1.f && m_scale.y == 1.f) {
    return m_position;
  }
  vec2 boundingBox = get_bounding_box();
  float x_offset, y_offset;
  y_offset = -screen.y * 0.5 + boundingBox.y * 1.5;
  if (is_left) {
    x_offset = -screen.x * 0.5 + boundingBox.x;
  } else {
    // ideally, it should just be a reverse of x_offset but not sure why - boundingBox.x * 0.5 does not work 
    x_offset = screen.x * 0.5 - boundingBox.x * 0.6;
  }
  vec2 position = { x_offset, y_offset };
  return position;
}
