#include "entity.hpp"
#include "movable_wall.hpp"
#include "CollisionManager.hpp"
#include "player.hpp"

#include <iostream>

#define BLOCK_SIZE 64

bool MovableWall::init(float xPos, float yPos)
{
	initial_position = { xPos, yPos };
	currentTime = 0.f;
	return Wall::init(xPos, yPos);
}

void MovableWall::set_movement_properties(
	bool shouldCurve,
	std::vector<vec2> blockLocations,
	std::vector<vec2> curveLocations,
	float speed,
	bool moving_immediately,
	bool loop_movement,
	bool loop_reverses)
{
	targetBlockLocations = blockLocations;
	curveBlockLocations = curveLocations;
	curving = shouldCurve;

	move_speed = speed;

	can_move = true;
	movementLoops = loop_movement;
	reverseWhenLooping = loop_reverses;
	isReversed = false;

	if (moving_immediately)
	{
	    activate();
	}
}

void MovableWall::activate() {
	if (can_move) {
		if (is_moving) { // if it's currently moving it is going in reverse from being deactivated so it won't need to travel the entire distance
			currentTime = (msToDestination - currentTime) + timeAtLastPoint;
		}
		is_moving = true;
		isReversed = false;
		currentTargetIndex = -1;
		AdvanceToNextPoint();
	}
}

void MovableWall::deactivate() {
	if (can_move) {
		if (is_moving) {
			//we want the time since timeAtLastPoint to be the same as the time until it hits its current destination
			// time until it hits its current destination: msToDestination - currentTime
			currentTime = (msToDestination - currentTime) + timeAtLastPoint;
		}
		is_moving = true;
		isReversed = true;
		currentTargetIndex = 0;
		AdvanceToNextPoint();
	}
}

void MovableWall::AdvanceToNextPoint()
{
	int nextIndex = currentTargetIndex + (isReversed ? -1 : 1);

	if (currentTargetIndex != -1)
	{
		previousLocation = targetBlockLocations[currentTargetIndex] * BLOCK_SIZE;
	}
	else
	{
		previousLocation = initial_position;
	}

	int size = targetBlockLocations.size();

	if (-1 > nextIndex)
	{
		isReversed = false;
		nextIndex = 0;

		if (!movementLoops) {
			is_moving = false;
			return;
		}
	}
	else if (nextIndex >= size)
	{
		if (!movementLoops)
		{
			is_moving = false;
			return;
		}
		else if (reverseWhenLooping)
		{
			isReversed = true;
			nextIndex = nextIndex - 2;
		}
		else
		{
			nextIndex = 0;
		}
	}

	currentTargetIndex = nextIndex;

	timeAtLastPoint = currentTime;
	currentTargetLocation = currentTargetIndex == -1 ? initial_position : targetBlockLocations[currentTargetIndex] * BLOCK_SIZE;
	float distanceToTarget;
	if (curving)
	{
		int curvePointIndex = isReversed ? currentTargetIndex + 1: currentTargetIndex;
		currentCurvePoint = curveBlockLocations[curvePointIndex] * BLOCK_SIZE;
		vec2 toCurvePoint = currentCurvePoint - get_position();
		vec2 toEnd = currentTargetLocation - currentCurvePoint;

		distanceToTarget = toCurvePoint.Magnitude() + toEnd.Magnitude();
	}
	else
	{
		distanceToTarget = (currentTargetLocation - previousLocation).Magnitude();
	}

	msToDestination = distanceToTarget / move_speed;
}

void MovableWall::update(float ms) {
	Entity::update(ms);
	currentTime += ms;

	if (is_moving) {
		vec2 pos = get_position();

		float move_dest_X = currentTargetLocation.x;
		float move_dest_Y = currentTargetLocation.y;

		// calculate dist to move_dest
		float x_dist = move_dest_X - pos.x;
		float y_dist = move_dest_Y - pos.y;

		float dest_distance = sqrt((x_dist*x_dist) + (y_dist*y_dist));

		if (dest_distance == 0)
		{
			return;
		}

		float x_normalized = x_dist / dest_distance;
		float y_normalized = y_dist / dest_distance;

		vec2 newPos;
		// if true, the block will move past the destination this tick, so do action for when block reaches the end of its path
		float timeDiff = currentTime - timeAtLastPoint;
		if (timeDiff > msToDestination)
		{
			newPos = { currentTargetLocation.x, currentTargetLocation.y };
			AdvanceToNextPoint();
		}
		else {
			if (!curving)
			{
				newPos = { pos.x + (x_normalized * move_speed * ms), pos.y + (y_normalized * move_speed * ms) };

				// Hacky fix to terminate movement early if we're close enough to the
				// target location, regardless of the msToDestination calculation.
				if (!movementLoops && std::abs(newPos.x - currentTargetLocation.x) < 1 && std::abs(newPos.y - currentTargetLocation.y) < 1) {
					// std::cout << "Stopped movement early with hacky logic" << std::endl;
					is_moving = false;
					newPos.x = currentTargetLocation.x;
					newPos.y = currentTargetLocation.y;
				}
			}
			else
			{
				float timeFrac = timeDiff / msToDestination;
				float oneMinusTimeFrac = 1 - timeFrac;

				vec2 curvePath = previousLocation * oneMinusTimeFrac * oneMinusTimeFrac + currentCurvePoint * 2 * oneMinusTimeFrac * timeFrac + currentTargetLocation * timeFrac * timeFrac;
				newPos = curvePath;
			}

			velocity.x = newPos.x - pos.x;
			velocity.y = newPos.y - pos.y;
		}

		vec2 movement = newPos - pos;
		CollisionManager::CollisionResult collisionResult;
		bool collidesWithPlayer = CollisionManager::GetInstance().CollidesWithPlayer(pos, get_bounding_box(), movement, collisionResult);

		if (collidesWithPlayer)
		{
			CollisionManager::GetInstance().MovePlayer({ collisionResult.resultXPos, collisionResult.resultYPos });
		}

		set_position(newPos);
	}
	else {
		velocity.x = 0;
		velocity.y = 0;
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

vec2 MovableWall::get_velocity()
{
	return velocity;
}
