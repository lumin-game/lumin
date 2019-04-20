#pragma once

#include <ctime>
#include "entity.hpp"
#include "wall.hpp"
#include "common.hpp"

class MovableWall : public Wall {
public:
	bool init(float xPos, float yPos) override;

	void update(float ms) override;
	void activate() override;
	void deactivate() override;

	bool is_light_dynamic() const override { return true; }
	virtual bool activated_by_light() const override { return false; }


	void set_movement_properties(bool shouldCurve, std::vector<vec2> blockLocations, std::vector<vec2> curveLocations, float speed, bool moving_immediately, bool loop_movement, bool loop_reverses);

	ParametricLines calculate_static_equations() const override;
	ParametricLines calculate_dynamic_equations() const override;

	vec2 get_velocity();

private:
	void AdvanceToNextPoint();

	bool curving;
	std::vector<vec2> targetBlockLocations;
	std::vector<vec2> curveBlockLocations;

	vec2 initial_position;

	int currentTargetIndex;
	vec2 currentTargetLocation;
	vec2 previousLocation;
	vec2 currentCurvePoint;

	float move_speed;
	float msToDestination;
	float timeAtLastPoint;

	bool can_move;
	bool is_moving;

	bool movementLoops;
	bool reverseWhenLooping;
	bool isReversed;

	float currentTime;

	vec2 velocity;
};
