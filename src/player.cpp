#include "player.hpp"

// internal
#include "entity.hpp"
#include "world.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

bool Player::init(const World* world)
{
	playerMesh.init();
	lightMesh.init(world);

	playerWidth = playerMesh.GetPlayerWidth();
	playerHeight = playerMesh.GetPlayerHeight();

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 0.5f;
	m_scale.y = 0.5f;
	m_position = { 100.f, 50.f };

	m_x_velocity = 0;
	m_y_velocity = 0;

	can_jump = false;

	return true;
}

// Releases all graphics resources
void Player::destroy()
{
	playerMesh.destroy();
	lightMesh.destroy();
}

// Called on each frame by World::update()
void Player::update(float ms)
{
	const float WALK_SPEED = 60.f;
	const float GROUND_FRICTION = 45.f;
	const float GRAVITY = 22.f;
	float gravity_step = GRAVITY * (ms / 1000);
	float x_velocity_step = WALK_SPEED * (ms / 1000);
	float friction_step = GROUND_FRICTION * (ms / 1000);

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Update player position/velocity based on key presses
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	if (m_is_z_pressed && can_jump) {
		m_y_velocity = -8.f;
	}

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
	if (m_x_velocity > 5.f) {
		m_x_velocity = 5.f;
	}
	else if (m_x_velocity < -5.f) {
		m_x_velocity = -5.f;
	}

	m_y_velocity += gravity_step;

	m_position.x += m_x_velocity * (ms/10);
	m_position.y += m_y_velocity * (ms/10);

	m_screen_pos.y = m_position.y;
	float screen_width = 64*16;
	if (m_position.x >= screen_width * 3/4) {
		m_screen_pos.x = screen_width * 3/4;
	} else if (m_position.x <= screen_width * 1/4) {
		m_screen_pos.x = screen_width * 1/4;
	} else {
		m_screen_pos.x = m_position.x;
	}

	can_jump = false;
}

void Player::draw(const mat3& projection)
{
	LightMesh::ParentData lightData;
	lightData.m_position = m_position;
	lightData.m_screen_pos = m_screen_pos;

	lightMesh.SetParentData(lightData);
	lightMesh.draw(projection);

	PlayerMesh::ParentData playerData;
	playerData.m_position = m_position;
	playerData.m_screen_pos = m_screen_pos;

	playerMesh.SetParentData(playerData);
	playerMesh.draw(projection);
}

// Simple bounding box collision check,
bool Player::collides_with(Entity& entity) {
	float entity_left = entity.get_position().x;
	float entity_top = entity.get_position().y;
	float entity_right = entity_left + entity.get_bounding_box().x;
	float entity_bottom = entity_top + entity.get_bounding_box().y;

	float player_left = m_position.x;
	float player_top = m_position.y;
	float player_right = player_left + (m_scale.x * playerWidth);
	float player_bottom = player_top + (m_scale.y * playerHeight);

	float dist_passed_top = player_bottom - entity_top;
	float dist_passed_bottom = entity_bottom - player_top;
	float dist_passed_left = player_right - entity_left;
	float dist_passed_right = entity_right - player_left;

	//Determine whether player rectangle and entity rectangle overlap
	bool rects_overlap = true;
	if (player_left >= entity_right || entity_left >= player_right) {
		rects_overlap = false;
	}

	// If one rectangle is above other
	if (player_top >= entity_bottom || entity_top >= player_bottom) {
		rects_overlap = false;
	}

	//TODO update this following part to actually work lol

	//New attempt at collision detection
	if (rects_overlap) {
		if (m_x_velocity == 0) {
			if (m_y_velocity >= 0) {
				//player is going straight downwards so move to top of block
				m_position.y = entity_top - (m_scale.y * playerHeight);
				m_y_velocity = 0.05f;
				can_jump = true;
			}
			else {
				//player is going straight upwards so move to bottom of block
				m_position.y = entity_bottom;
				m_y_velocity = -0.05f;
			}
		}
		else if (m_y_velocity == 0) {
			if (m_x_velocity >= 0) {
				//player is going straight rightwards so move to left of block
				m_position.x = entity_left - (m_scale.x  * playerWidth);
				m_x_velocity = 0.05f;
			}
			else {
				//player is going straight leftwards to move to right of block
				m_position.x = entity_right;
				m_x_velocity = -0.05f;
			}
		}
		else { //player is moving in both x and y
			if (m_x_velocity > 0 && m_y_velocity > 0) {
				// player is moving down right, so move the player to either the left or top of the platform, whichever is closer

				if (dist_passed_top <= dist_passed_left) {
					m_position.y = entity_top - (m_scale.y * playerHeight);
					m_y_velocity = 0.05f;
					can_jump = true;
				}
				else {
					m_position.x = entity_left - (m_scale.x * playerWidth);
					m_x_velocity = 0.05f;
				}
			}
			if (m_x_velocity > 0 && m_y_velocity < 0) {
				// player is moving up right, so move the player to either the left or bottom of the platform, whichever is closer
				if (dist_passed_bottom <= dist_passed_left) {
					m_position.y = entity_bottom;
					m_y_velocity = -0.05f;
				}
				else {
					m_position.x = entity_left - (m_scale.x * playerWidth);
					m_x_velocity = 0.05f;
				}
			}
			if (m_x_velocity < 0 && m_y_velocity > 0) {
				// player is moving down left, so move the player to either the right or top of the platform, whichever is closer
				if (dist_passed_top <= dist_passed_right) {
					m_position.y = entity_top - (m_scale.y * playerHeight);
					m_y_velocity = 0.05f;
					can_jump = true;
				}
				else {
					m_position.x = entity_right;
					m_x_velocity = -0.05f;
				}
			}
			if (m_x_velocity < 0 && m_y_velocity < 0) {
				// player is moving up left, so move the player to either the right or bottom of the platform, whichever is closer
				if (dist_passed_bottom <= dist_passed_right) {
					m_position.y = entity_bottom;
					m_y_velocity = -0.05f;
				}
				else {
					m_position.x = entity_right;
					m_x_velocity = -0.05f;
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

vec2 Player::get_screen_pos()const
{
	return m_screen_pos;
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
	playerMesh.turn_left();
}

void Player::setRightPressed(bool tf) {
	m_is_right_pressed = tf;
	playerMesh.turn_right();
}
