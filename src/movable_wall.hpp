#pragma once

#include "entity.hpp"
#include "wall.hpp"
#include "common.hpp"

class MovableWall : public Wall {
public:
	~MovableWall() override { Entity::destroy(); }

	void set_movement_properties(int move_blocks_X, int move_blocks_Y, float speed, bool moving_immediately, bool loop_movement);

	void trigger_movement(bool moving);

	void update(float ms);

private:
	float move_dest_X;
	float move_dest_Y;
	float move_speed;
	bool is_moving;
	bool movement_loops;
	bool movement_direction;
	vec2 initial_position;
};