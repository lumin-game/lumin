#pragma once

#include "entity.hpp"
#include "wall.hpp"
#include "common.hpp"

class MovableWall : public Wall {
public:
	void update(float ms) override;
	void activate() override;

	bool is_light_dynamic() const override { return true; }

	void set_movement_properties(float deltaX, float deltaY, float speed, bool moving_immediately, bool loop_movement);

	ParametricLines calculate_static_equations() const override;
	ParametricLines calculate_dynamic_equations() const override;

private:
	float move_dest_X;
	float move_dest_Y;
	float move_blocks_X;
	float move_blocks_Y;
	float move_speed;
	bool is_moving;
	bool movement_loops;
	bool moving_forward;
	vec2 initial_position;
	bool can_move;
};
