#include "LightWall.hpp"
#include "CollisionManager.hpp"

void LightWall::activate()
{
	set_lit(true);
	if (CollisionManager::GetInstance().BoxCollideWithPlayer(m_position, get_bounding_box()))
	{
		shouldBeCollidable = true;
	}
	else
	{
		isCollidable = true;
	}
}

void LightWall::deactivate()
{
	set_lit(false);
	isCollidable = false;
	shouldBeCollidable = false;
}

void LightWall::update(float ms)
{
	if (shouldBeCollidable)
	{
		if (!CollisionManager::GetInstance().BoxCollideWithPlayer(m_position, get_bounding_box()))
		{
			isCollidable = true;
			shouldBeCollidable = false;
		}
	}
}