  // Header
#include "player.hpp"

// internal
#include "wall.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

Texture Player::player_texture;


bool Player::init()
{
	if (!player_texture.is_valid())
	{
		if (!player_texture.load_from_file(textures_path("fish.png")))
		{
			fprintf(stderr, "Failed to load player texture!");
			return false;
		}
	}
	// The position corresponds to the center of the texture
	float wr = player_texture.width * 0.5f;
	float hr = player_texture.height * 0.5f;

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
	m_scale.x = 0.4f;
	m_scale.y = 0.4f;
	m_position = { 50.f, 100.f };

	m_x_velocity = 0;
	m_y_velocity = 0;

	return true;
}

// Releases all graphics resources
void Player::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

// Called on each frame by World::update()
void Player::update(float ms)
{
	const float WALK_SPEED = 60.f;
	const float GROUND_FRICTION = 30.f;
	const float GRAVITY = 15.f;
	float gravity_step = GRAVITY * (ms / 1000);
	float x_velocity_step = WALK_SPEED * (ms / 1000);
	float friction_step = GROUND_FRICTION * (ms / 1000);

	
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// Update player position/velocity based on key presses
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
	
	if (m_is_left_pressed) {
		m_x_velocity -= x_velocity_step;
	} else if (m_is_right_pressed) {
		m_x_velocity += x_velocity_step;
	}
	else {
		if (m_x_velocity > 0) {
			m_x_velocity -= std::min(m_x_velocity, friction_step);
		}
		else if (m_x_velocity < 0) {
			m_x_velocity += std::min(m_x_velocity * -1.f, friction_step);
		}
	}

	m_x_velocity = std::min(m_x_velocity, 10.f);
	m_x_velocity = std::max(m_x_velocity, -10.f);

	m_y_velocity += gravity_step;

	m_position.x += m_x_velocity;
	m_position.y += m_y_velocity;
}

void Player::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()


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
	glBindTexture(GL_TEXTURE_2D, player_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 0.f, 0.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

// Simple bounding box collision check, 
bool Player::collides_with(const Wall& wall)
{
	float wall_left = wall.get_position().x;
	float wall_top = wall.get_position().y;
	float wall_right = wall_left + wall.get_bounding_box().x;
	float wall_bottom = wall_top + wall.get_bounding_box().y;

	float player_left = m_position.x;
	float player_top = m_position.y;
	float player_right = player_left + (m_scale.x * player_texture.width);
	float player_bottom = player_top + (m_scale.y * player_texture.height);

	float dist_passed_top = player_bottom - wall_top;
	float dist_passed_bottom = wall_bottom - player_top;
	float dist_passed_left = player_right - wall_left;
	float dist_passed_right = wall_right - player_left;

	//Determine whether player rectangle and wall rectangle overlap
	bool rects_overlap = true;
	if (player_left > wall_right || wall_left > player_right) {
		rects_overlap = false;
	}

	// If one rectangle is above other 
	if (player_top > wall_bottom || wall_top > player_bottom) {
		rects_overlap = false;
	}

	//TODO update this following part to actually work lol

	//New attempt at collision detection
	if (rects_overlap) {
		if (m_x_velocity == 0) {
			if (m_y_velocity >= 0) {
				//player is going straight downwards so move to top of block
				m_position.y = wall_top - (m_scale.y * player_texture.height);
				m_y_velocity = 0.05f;
			}
			else {
				//player is going straight upwards so move to bottom of block
				m_position.y = wall_bottom;
				m_y_velocity = -0.05f;
			}
		}
		else if (m_y_velocity == 0) {
			if (m_x_velocity >= 0) {
				//player is going straight rightwards so move to left of block
				m_position.x = wall_left - (m_scale.x * player_texture.width);
				m_x_velocity = 0.05f;
			}
			else {
				//player is going straight leftwards to move to right of block
				m_position.x = wall_right;
				m_y_velocity = -0.05f;
			}
		}
		else { //player is moving in both x and y
			if (m_x_velocity > 0 && m_y_velocity > 0) {
				// player is moving down right, so move the player to either the left or top of the platform, whichever is closer
				std::cout << "in correct case" << std::endl;
				if (dist_passed_top <= dist_passed_left) {
					m_position.y = wall_top - (m_scale.y * player_texture.height);
					m_y_velocity = 0;
				}
				else {
					m_position.x = wall_left - (m_scale.x * player_texture.width);
					m_x_velocity = 0;
				}
			}
			if (m_x_velocity > 0 && m_y_velocity < 0) {
				// player is moving up right, so move the player to either the left or bottom of the platform, whichever is closer 
				if (dist_passed_bottom <= dist_passed_left) {
					m_position.y = wall_bottom;
					m_y_velocity = 0;
				}
				else {
					m_position.x = wall_left - (m_scale.x * player_texture.width);
					m_x_velocity = 0;
				}
			}
			if (m_x_velocity < 0 && m_y_velocity > 0) {
				// player is moving down left, so move the player to either the right or top of the platform, whichever is closer 
				if (dist_passed_top <= dist_passed_right) {
					m_position.y = wall_top - (m_scale.y * player_texture.height);
					m_y_velocity = 0;
				}
				else {
					m_position.x = wall_right;
					m_x_velocity = 0;
				}
			}
			if (m_x_velocity < 0 && m_y_velocity < 0) {
				// player is moving up left, so move the player to either the right or bottom of the platform, whichever is closer 
				if (dist_passed_bottom <= dist_passed_right) {
					m_position.y = wall_bottom;
					m_y_velocity = 0;
				}
				else {
					m_position.x = wall_right;
					m_x_velocity = 0;
				}
			}
		}
		//TODO!!!!!: This will cause the player to stutter sometimes when going over connections between 2 blocks
		// UNLESS I find a way to make it choose the secondary choice in all of the cases where velX and velY != 0 if the primary choice
		// would result in the player colliding with a different block

	}

	return false;
}

vec2 Player::get_position()const
{
	return m_position;
}

void Player::move(vec2 off)
{
	m_position.x += off.x; m_position.y += off.y;
}

void Player::setZPressed(bool tf) {
	m_is_z_pressed = tf;
}

void Player::setLeftPressed(bool tf) {
	m_is_left_pressed = tf;
}

void Player::setRightPressed(bool tf) {
	m_is_right_pressed = tf;
}