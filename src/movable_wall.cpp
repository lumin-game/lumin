#include "entity.hpp"
#include "movable_wall.hpp"

#include <iostream>

void MovableWall::set_movement_properties(float move_blocks_X, float move_blocks_Y, float speed, bool moving_immediately, bool loop_movement) {
	const uint32_t BLOCK_SIZE = 64;
	
	initial_position = get_position();
	move_dest_X = initial_position.x + (move_blocks_X * BLOCK_SIZE);
	move_dest_Y = initial_position.y + (move_blocks_Y * BLOCK_SIZE);
	move_speed = speed;
	is_moving = moving_immediately;
	movement_loops = loop_movement;
	moving_forward = true;
}

void MovableWall::trigger_movement(bool moving) {
	is_moving = moving;
}

void MovableWall::update(float ms) {
	if (is_moving) {
		vec2 pos = get_position();

		float x_dist;
		float y_dist;

		// calculate dist to move_dest if move dir is true (moving forwards), or to initial position if move dir is false (moving backwards)
		if (moving_forward) {
			x_dist = move_dest_X - pos.x;
			y_dist = move_dest_Y - pos.y;
		}
		else {
			x_dist = initial_position.x - pos.x;
			y_dist = initial_position.y - pos.y;
		}
		
		float dest_distance = sqrt((x_dist*x_dist) + (y_dist*y_dist));
		float x_normalized = x_dist / dest_distance;
		float y_normalized = y_dist / dest_distance;

		// if true, the block will move past the destination this tick, so do action for when block reaches the end of its path
		if (abs(x_normalized * move_speed * ms) > abs(x_dist) || abs(y_normalized * move_speed * ms) > abs(y_dist)) {

			//move block to its destination
			if (moving_forward) {
				set_position({ move_dest_X, move_dest_Y });
			}
			else {
				set_position({ initial_position.x, initial_position.y });
			}
			
			if (movement_loops) {
				moving_forward = !moving_forward;
			}
			else {
				is_moving = false;
			}
		}
		else {
			set_position({ pos.x + (x_normalized * move_speed * ms), pos.y + (y_normalized * move_speed * ms) });
		}
	}
}

ParametricLines MovableWall::calculate_static_equations() const
{
	return ParametricLines();
}

ParametricLines MovableWall::calculate_dynamic_equations() const
{
	return Entity::calculate_static_equations();
}
