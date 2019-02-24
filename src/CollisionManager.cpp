#include <memory>
#include <cmath>
#include <iostream>
#include "CollisionManager.hpp"


void CollisionManager::RegisterEntity(const Entity* entity)
{
	if (staticLightCollisionLines.find(entity) != staticLightCollisionLines.end())
	{
		// Already exists, remove the old one
		UnregisterEntity(entity);
	}

	staticLightCollisionLines.emplace(entity, entity->calculate_static_equations());

	if (entity->is_player_collidable())
	{
		staticCollisionEntities.push_back(entity);
	}
}

void CollisionManager::UnregisterEntity(const Entity* entity)
{
	staticLightCollisionLines.erase(entity);
	for (auto iter = staticCollisionEntities.begin(); iter != staticCollisionEntities.end(); ++iter)
	{
		if (*iter == entity)
		{
			staticCollisionEntities.erase(iter);
			break;
		}
	}
}

const CollisionManager::CollisionResult CollisionManager::BoxTrace(int width, int height, float xPos, float yPos, float xDist, float yDist, bool unstoppable) const
{
	CollisionResult collisionResults;
	collisionResults.resultXPos = xPos + xDist;
	collisionResults.resultYPos = yPos + yDist;
	for (const Entity* entity : staticCollisionEntities)
	{
		// Center-to-center distance between two boxes
		float distanceX = std::fabs(entity->get_position().x - xPos - xDist);
		float distanceY = std::fabs(entity->get_position().y - yPos - yDist);

		// Margin is how much distance can be between the two centers before collision
		float xMargin = (entity->get_bounding_box().x + width) / 2;
		float yMargin = (entity->get_bounding_box().y + height) / 2;

		if (distanceX < xMargin && distanceY < yMargin)
		{
			EntityResult entityResult;
			entityResult.entity = entity;
			entityResult.xPos = entity->get_position().x;
			entityResult.yPos = entity->get_position().y;

			// yPos wasn't in the margin, but after moving yDist it will be
			if (std::fabs(entity->get_position().y - yPos) < yMargin)
			{
				// We know we will collide in the X axis.
				float margin = xDist > 0 ? -xMargin : xMargin;

				if (unstoppable)
				{
					//TODO: this is for moving platforms colliding into players. UNTESTED!!
					entityResult.xPos = xPos + margin;
				}
				else
				{
					collisionResults.resultXPos = entity->get_position().x + margin;
					xDist = collisionResults.resultXPos - xPos;
				}
			}
			else
			{
				// We know we will collide in the Y axis.
				float margin = yDist > 0 ? -yMargin : yMargin;
				if (yDist > 0)
				{
					collisionResults.hitGround = true;
				}
				else
				{
					collisionResults.hitCeiling = true;
				}

				if (unstoppable)
				{
					//TODO: this is for moving platforms colliding into players. UNTESTED!!
					entityResult.yPos = yPos + margin;
				}
				else
				{
					collisionResults.resultYPos = entity->get_position().y + margin;
					yDist = collisionResults.resultYPos - yPos;
				}
			}

			collisionResults.entitiesHit.push_back(entityResult);
		}
	}

	return collisionResults;
}

const ParametricLines CollisionManager::CalculateLightEquations(float xPos, float yPos, float lightRadius) const
{
	ParametricLines outEquations;
	for (std::pair<const Entity*, ParametricLines> entry : staticLightCollisionLines)
	{
		const Entity* entity = entry.first;
		// Center-to-center distance between two boxes
		float distanceX = fmax(0.f, std::fabs(entity->get_position().x - xPos) - entity->get_bounding_box().x / 2);
		float distanceY = fmax(0.f, std::fabs(entity->get_position().y - yPos) - entity->get_bounding_box().y / 2);
		float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

		if (distance < lightRadius)
		{
			for (ParametricLine staticLine : entry.second)
			{
				// Since only position is at play, (and no scaling)
				// We only have to do a simple translation
				ParametricLine collEq;
				collEq.x_0 = staticLine.x_0 - xPos;
				collEq.x_t = staticLine.x_t;
				collEq.y_0 = staticLine.y_0 - yPos;
				collEq.y_t = staticLine.y_t;

				outEquations.push_back(collEq);
			}
		}
	}

	return outEquations;
}

bool CollisionManager::IsHitByLight(const Entity* entity, const Player* player, float lightRadius) const {
	// Center-to-center distance between two boxes
	float distanceX = fmax(0.f, std::fabs(entity->get_position().x - player->get_position().x) - entity->get_bounding_box().x / 2);
	float distanceY = fmax(0.f, std::fabs(entity->get_position().y - player->get_position().y) - entity->get_bounding_box().y / 2);
	float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

	return distance < lightRadius;
}