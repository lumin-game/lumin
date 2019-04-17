#include "DarkWall.hpp"
#include "CollisionManager.hpp"

void DarkWall::deactivate()
{
	set_lit(false);
	if (CollisionManager::GetInstance().BoxCollideWithPlayer(m_position, get_bounding_box()))
	{
		shouldBeCollidable = true;
	}
	else
	{
		isCollidable = true;
	}
}

void DarkWall::activate()
{
	set_lit(true);
	isCollidable = false;
	shouldBeCollidable = false;
}

void DarkWall::update(float ms)
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