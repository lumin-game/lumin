#pragma once

#include "player_mesh.hpp"
#include "light_mesh.hpp"
#include "common.hpp"

class World;

class Player
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update player position
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the player
	void draw(const mat3& projection, const float screen_w, const float screen_h);

	// Returns the current player position
	vec2 get_position()const;

	vec2 get_screen_pos()const;

	// Moves the player's position by the specified offset
	void move(vec2 off);

	void setLeftPressed(bool tf);

	void setRightPressed(bool tf);

	void setZPressed(bool tf);

private:
	vec2 m_position; // Window coordinates
	vec2 m_screen_pos; // Position on screen
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture

	bool m_is_left_pressed;
	bool m_is_right_pressed;
	bool m_is_z_pressed;

	bool can_jump;

	float m_y_velocity;
	float m_x_velocity;
	float m_max_fall_velocity;

	float m_screen_x_movement;
	float m_screen_y_movement;

	int playerWidth;
	int playerHeight;

	PlayerMesh playerMesh;
	LightMesh lightMesh;
};
