  // Header
#include "player_mesh.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>

Texture PlayerMesh::player_spritesheet;

bool PlayerMesh::init()
{
	m_current_frame = 0;
	m_frame_counter = 0;

    if (!player_spritesheet.is_valid())
    {
        if (!player_spritesheet.load_from_file(spritesheet_path("player.png")))
        {
            fprintf(stderr, "Failed to load player spritesheet!");
            return false;
        }
    }
	// The position corresponds to the center of the texture
	float wr = 125 * 0.5f;
	float hr = player_spritesheet.height * 0.5f;

	float tex_right = 1.f / TOTAL_FRAMES;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, 0.1f };
	vertices[1].position = { +wr, +hr, 0.1f };
	vertices[2].position = { +wr, -hr, 0.1f };
	vertices[3].position = { -wr, -hr, 0.1f };

	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].texcoord = { tex_right, 1.f };
	vertices[2].texcoord = { tex_right, 0.f };
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
	if (!effect.load_from_file(shader_path("player.vs.glsl"), shader_path("player.fs.glsl")))
		return false;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 0.45f;
	m_scale.y = 0.45f;

	return true;
}

// Render player as facing right
void PlayerMesh::turn_right()
{
	m_scale.x = std::fabs(m_scale.x);
}

// Render player as facing left
void PlayerMesh::turn_left()
{
	m_scale.x = -std::fabs(m_scale.x);
}

// Releases all graphics resources
void PlayerMesh::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void PlayerMesh::draw(const mat3& projection)
{

    // The position corresponds to the center of the texture
    float wr = 125 * 0.5f;
    float hr = player_spritesheet.height * 0.5f;

    // move to correct sprite on spritesheet
    TexturedVertex vertices[4];
    float tex_left = (float) m_current_frame / TOTAL_FRAMES;
    float tex_right = (m_current_frame + 1.f) / TOTAL_FRAMES;
    vertices[1].position = { +wr, +hr, 0.1f };
    vertices[0].position = { -wr, +hr, 0.1f };
    vertices[2].position = { +wr, -hr, 0.1f };
    vertices[3].position = { -wr, -hr, 0.1f };

    vertices[0].texcoord = { tex_left, 1.f };
    vertices[1].texcoord = { tex_right, 1.f };
    vertices[2].texcoord = { tex_right, 0.f };
    vertices[3].texcoord = { tex_left, 0.f };

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	transform_begin();
	transform_translate(m_parent.m_position);
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
	GLint light_radius = glGetUniformLocation(effect.program, "lightRadius");

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
	glBindTexture(GL_TEXTURE_2D, player_spritesheet.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 0.f, 0.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	glUniform1f(light_radius, 200.f);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

int PlayerMesh::GetPlayerHeight() const
{
	return player_spritesheet.height;
}

int PlayerMesh::GetPlayerWidth() const
{
	return player_spritesheet.width / TOTAL_FRAMES;

}

  void PlayerMesh::updateFrame()
  {
	if (m_frame_counter == (FRAME_SPEED - 1)) {
		m_current_frame = (m_current_frame + 1) % TOTAL_FRAMES;
	}
	m_frame_counter = (m_frame_counter + 1) % FRAME_SPEED;
  }
