#include "player.hpp"

// internal
#include "entity.hpp"
#include "world.hpp"
#include "CollisionManager.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

bool Player::init()
{
	playerMesh.init();
	lightMesh.init();

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture
	m_scale.x = 0.5f;
	m_scale.y = 0.5f;

	playerWidth = (int)(playerMesh.GetPlayerWidth() * m_scale.x);
	playerHeight = (int)(playerMesh.GetPlayerHeight() * m_scale.y);
	m_screen_x_movement = 0.f;
	m_screen_y_movement = 0.f;
  
	m_x_velocity = 0;
	m_y_velocity = 0; 
	m_max_fall_velocity = 20.f;

	can_jump = false;

	CollisionManager::GetInstance().RegisterPlayer(this);
	return true;
}

// Releases all graphics resources
void Player::destroy()
{
	playerMesh.destroy();
	lightMesh.destroy();

	CollisionManager::GetInstance().UnregisterPlayer();
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
		m_is_z_pressed = false;
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
	m_y_velocity = std::min(m_y_velocity, m_max_fall_velocity);

	// distances are how much we plan to move this frame
	float xDist = m_x_velocity * (ms / 10);
	float yDist = m_y_velocity * (ms / 10);
	CollisionManager::CollisionResult collisionResult = CollisionManager::GetInstance().BoxTrace(playerWidth, playerHeight, m_position.x, m_position.y, xDist, yDist);

	// calculate how much player moved during this update to figure out how much player should move on screen
	m_screen_x_movement = collisionResult.resultXPos - m_position.x;
	m_screen_y_movement = collisionResult.resultYPos - m_position.y;

	m_position.x = collisionResult.resultXPos;
	m_position.y = collisionResult.resultYPos;
	can_jump = collisionResult.bottomCollision;

	if (collisionResult.bottomCollision || collisionResult.topCollision) 
	{
		m_y_velocity = collisionResult.resultYPush;
	}

	if(m_screen_x_movement != 0.f && m_screen_y_movement == 0.f)
	{
		// only update player animation if player is walking along horizontal surface
		playerMesh.updateFrame();
	}
}

void Player::draw(const mat3& projection)
{
	LightMesh::ParentData lightData;
	lightData.m_position = m_position;

	lightMesh.SetParentData(lightData);
	lightMesh.draw(projection);

	PlayerMesh::ParentData playerData;
	playerData.m_position = m_position;

	playerMesh.SetParentData(playerData);
	playerMesh.draw(projection);
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
	if (tf) {
		playerMesh.turn_left();
	}
}

void Player::setRightPressed(bool tf) {
	m_is_right_pressed = tf;
	if (tf) {
		playerMesh.turn_right();
	}
}

void Player::setPlayerPosition(vec2 pos) {
	m_position = pos;
}
