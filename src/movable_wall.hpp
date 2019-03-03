#pragma once

#include "entity.hpp"
#include "wall.hpp"
#include "common.hpp"

class MovableWall : public Wall {
public:
	~MovableWall() override { Entity::destroy(); }

	void set_movement_properties(float move_blocks_X, float move_blocks_Y, float speed, bool moving_immediately, bool loop_movement);

	void trigger_movement(bool moving);

	void update(float ms);

	void activate() override { trigger_movement(true); }

	bool is_light_dynamic() const override { return true; }

	ParametricLines calculate_static_equations() const override;
	ParametricLines calculate_dynamic_equations() const override;

private:
	float move_dest_X;
	float move_dest_Y;
	float move_speed;
	bool is_moving;
	bool movement_loops;
	bool moving_forward;
	vec2 initial_position;
	bool can_move;
};