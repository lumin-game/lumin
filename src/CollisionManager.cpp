#include <memory>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "CollisionManager.hpp"
#include "player.hpp"
#include "movable_wall.hpp"

void CollisionManager::RegisterPlayer(Player* playerPtr)
{
	player = playerPtr;
}

void CollisionManager::UnregisterPlayer()
{
	player = nullptr;
}

void CollisionManager::RegisterEntity(Entity* entity)
{
	if (registeredEntities.find(entity) != registeredEntities.end())
	{
		UnregisterEntity(entity);
	}

	registeredEntities.insert(entity);

	if (entity->is_light_collidable())
	{
		staticLightCollisionLines.emplace(entity, entity->calculate_static_equations());
	}
}

void CollisionManager::UnregisterEntity(Entity* entity)
{
	registeredEntities.erase(entity);

	staticLightCollisionLines.erase(entity);
	dynamicLightCollisionLines.erase(entity);
}

bool CollisionManager::CollidesWithPlayer(vec2 boxPosition, vec2 boxBound, vec2 boxDisplacement, CollisionResult& outResult) const
{
	if (player == nullptr)
	{
		return false;
	}

	float playerX = player->get_position().x;
	float playerY = player->get_position().y;
	float playerW = player->getPlayerDimensions().x;
	float playerH = player->getPlayerDimensions().y;

	// Center-to-center distance between two boxes
	float distanceX = std::fabs(boxPosition.x - playerX + boxDisplacement.x);
	float distanceY = std::fabs(boxPosition.y - playerY + boxDisplacement.y);

	// Margin is how much distance can be between the two centers before collision
	float xMargin = (boxBound.x + playerW) / 2;
	float yMargin = (boxBound.y + playerH) / 2;

	if (distanceX < xMargin && distanceY < yMargin)
	{
		// yPos wasn't in the margin, but after moving yDist it will be
		if (std::fabs(boxPosition.y - playerY) < yMargin)
		{
			// We know we will collide in the X axis.
			float margin = boxDisplacement.x < 0 ? -xMargin : xMargin;

			outResult.resultXPos = boxPosition.x + boxDisplacement.x + margin;
			outResult.resultYPos = playerY;
		}
		else
		{
			// We know we will collide in the Y axis.
			float margin = boxDisplacement.y < 0 ? -yMargin : yMargin;
			if (boxDisplacement.y > 0)
			{
				outResult.bottomCollision = true;
			}
			else
			{
				outResult.topCollision = true;
			}

			outResult.resultXPos = playerX;
			outResult.resultYPos = boxPosition.y + boxDisplacement.y + margin;
		}

		return true;
	}
	
	return false;
}

void CollisionManager::MovePlayer(vec2 movement)
{
	if (player == nullptr)
	{
		return;
	}

	player->setPlayerPosition(movement);
}

const CollisionManager::CollisionResult CollisionManager::BoxTrace(int width, int height, float xPos, float yPos, float xDist, float yDist) const
{
	CollisionResult collisionResults;
	collisionResults.resultXPos = xPos + xDist;
	collisionResults.resultYPos = yPos + yDist;

	std::vector<EntityDistance> collidingEntities;

	for (Entity* entity : registeredEntities)
	{
		if (!entity->is_player_collidable())
		{
			continue;
		}

		// Center-to-center distance between two boxes
		float distanceX = std::fabs(entity->get_position().x - xPos - xDist);
		float distanceY = std::fabs(entity->get_position().y - yPos - yDist);

		// Margin is how much distance can be between the two centers before collision
		float xMargin = (entity->get_bounding_box().x + width) / 2;
		float yMargin = (entity->get_bounding_box().y + height) / 2;

		if (distanceX < xMargin && distanceY < yMargin)
		{
			EntityDistance entDist;
			entDist.entity = entity;
			entDist.distanceSqr = distanceX * distanceX + distanceY * distanceY;
			collidingEntities.push_back(entDist);
		}
	}

	std::sort(collidingEntities.begin(), collidingEntities.end(), [](const EntityDistance ent1, const EntityDistance ent2)
	{
		return ent1.distanceSqr < ent2.distanceSqr;
	});

	for (struct EntityDistance colEntity : collidingEntities)
	{
		Entity* entity = colEntity.entity;
		// Center-to-center distance between two boxes
		float distanceX = std::fabs(entity->get_position().x - xPos - xDist);
		float distanceY = std::fabs(entity->get_position().y - yPos - yDist);

		// Margin is how much distance can be between the two centers before collision
		float xMargin = (entity->get_bounding_box().x + width) / 2;
		float yMargin = (entity->get_bounding_box().y + height) / 2;

		if (distanceX < xMargin && distanceY < yMargin)
		{
			float diffY = std::fabs(entity->get_position().y - yPos);
			float diffX = std::fabs(entity->get_position().x - xPos);

			// yPos wasn't in the margin, but after moving yDist it will be
			if (diffY < yMargin)
			{
				// We know we will collide in the X axis.
				float margin = xDist > 0 ? -xMargin : xMargin;

				collisionResults.resultXPos = entity->get_position().x + margin;
				xDist = collisionResults.resultXPos - xPos;	
			}
			else
			{
				// We know we will collide in the Y axis.

				MovableWall* mov_wall = dynamic_cast<MovableWall*>(entity);
				if (mov_wall != 0) { // if the pointer isn't null then player is intersecting with a moving block and they should travel with it
					collisionResults.resultXPos += mov_wall->get_velocity().x; // Drag the player in whatever X direction the block is moving
					if (mov_wall->get_velocity().y > 0) {
						// Make the player keep a similar Y velocity to the block so they will collide with it every frame and thus be dragged horizontally by it every frame
						collisionResults.resultYPush = mov_wall->get_velocity().y; 
					}
				}

				float margin = yDist > 0 ? -yMargin : yMargin;
				if (yDist > 0)
				{
					collisionResults.bottomCollision = true;
				}
				else
				{
					collisionResults.topCollision = true;
				}

				collisionResults.resultYPos = entity->get_position().y + margin;
				yDist = collisionResults.resultYPos - yPos;				
			}
		}
	}

	return collisionResults;
}

bool CollisionManager::BoxCollide(vec2 box1Pos, vec2 box1Bound, vec2 box2Pos, vec2 box2Bound) const {
	// Center-to-center distance between two boxes
	float distanceX = std::fabs(box1Pos.x - box2Pos.x);
	float distanceY = std::fabs(box1Pos.y - box2Pos.y);

	// Margin is how much distance can be between the two centers before collision
	float xMargin = (box1Bound.x + box2Bound.x) / 2;
	float yMargin = (box1Bound.y + box2Bound.y) / 2;

	return distanceX < xMargin && distanceY < yMargin;
}

bool CollisionManager::BoxCollideWithPlayer(vec2 boxPos, vec2 boxBound) const
{
	if (!player)
	{
		return false;
	}

	return BoxCollide(player->get_position(), player->getPlayerDimensions(), boxPos, boxBound);
}

const RadiusLightMesh* CollisionManager::GetPlayerRadiusLightMesh() const
{
    if (!player)
    {
        return nullptr;
    }

    return player->getPlayerRadiusLight();
}

const LaserLightMesh* CollisionManager::GetPlayerLaserLightMesh() const
{
    if (!player)
    {
        return nullptr;
    }

    return player->getPlayerLaserLight();
}

const std::vector<Entity*> CollisionManager::GetEntitiesInRange(float xPos, float yPos, float lightRadius) const
{
	std::vector<Entity*> outEntities;
	for (Entity* entity : registeredEntities)
	{
		const float xDiff = entity->get_position().x - xPos;
		const float yDiff = entity->get_position().y - yPos;
		const float xRadius = entity->get_bounding_box().x / 2;
		const float yRadius = entity->get_bounding_box().y / 2;
		float distanceX = fmax(0.f, std::fabs(xDiff) - xRadius);
		float distanceY = fmax(0.f, std::fabs(yDiff) - yRadius);
		float distanceSqr = distanceX * distanceX + distanceY * distanceY;
		
		if (distanceSqr < ((lightRadius)* lightRadius))
		{
			outEntities.push_back(entity);
		}
	}

	return outEntities;
}

bool CollisionManager::LinesCollide(ParametricLine line1, ParametricLine line2) const
{
	vec2 collisionPos;
	return LinesCollide(line1, line2, collisionPos);
}

bool CollisionManager::LinesCollide(ParametricLine line1, ParametricLine line2, vec2& collisionPos) const
{
	// Given
	// line1 : x1 = a1 + b1*t1, y1 = c1 + d1*t1
	// line2 : x2 = a2 + b2*t2, y2 = c2 + d2*t2
	// A collision means a pair of t1 and t2 that are both 0 < t < 1

	// Algebra gives:
	// t2 = (c1 - c2 + d1*a2/b1 - d1*a1/b1) / (d2 - d1*b2/b1)
	// t1 = (a2 + b2 * t2) / b1

	// However we must consider the case where b1 == 0
	// In that case we use the alternate equations
	// t2 = (a1-a2)/b2
	// t1 = (c2 - c1 + d2*t2) / d1

	float epsilon = 0.0001f;
	float a1 = line1.x_0;
	float b1 = line1.x_t;
	float c1 = line1.y_0;
	float d1 = line1.y_t;
	float a2 = line2.x_0;
	float b2 = line2.x_t;
	float c2 = line2.y_0;
	float d2 = line2.y_t;

	if (-epsilon < b1 && b1 < epsilon) // when b1 == 0
	{
		float t2 = (a1 - a2) / b2;
		float t1 = (c2 - c1 + d2*t2) / d1;

		if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
		{
			collisionPos = { a1 + b1 * t1, c1 + d1 * t1 };
			return true;
		}
	}
	else
	{
		float t2 = (c1 - c2 + d1 * a2 / b1 - d1 * a1 / b1) / (d2 - d1 * b2 / b1);
		float t1 = (a2 + b2 * t2) / b1;
		if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
		{
			collisionPos = { a1 + b1 * t1, c1 + d1 * t1 };
			return true;
		}
	}

	return false;
}

void CollisionManager::UpdateDynamicLightEquations()
{
	dynamicLightCollisionLines.clear();
	for (const Entity* entity : registeredEntities)
	{
		if (entity->is_light_collidable() && entity->is_light_dynamic())
		{
			dynamicLightCollisionLines.emplace(entity, entity->calculate_dynamic_equations());
		}
	}
}

const ParametricLines CollisionManager::CalculateLightEquations(float xPos, float yPos, float lightRadius) const
{
    ParametricLines outEquations;
    for (std::pair<const Entity*, ParametricLines> entry : staticLightCollisionLines)
    {
        CalculateLightEquationForEntry(entry, outEquations, xPos, yPos, lightRadius);
    }
    for (std::pair<const Entity*, ParametricLines> entry : dynamicLightCollisionLines)
    {
        CalculateLightEquationForEntry(entry, outEquations, xPos, yPos, lightRadius);
    }

    return outEquations;
}

void CollisionManager::CalculateLightEquationForEntry(std::pair<const Entity*, ParametricLines> entry, ParametricLines& outLines, float xPos, float yPos, float lightRadius) const
{
    const Entity* entity = entry.first;
    // Center-to-center distance between two boxes
    float distanceX = fmax(0.f, std::fabs(entity->get_position().x - xPos) - entity->get_bounding_box().x / 2);
    float distanceY = fmax(0.f, std::fabs(entity->get_position().y - yPos) - entity->get_bounding_box().y / 2);
    float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

    if (distance < lightRadius)
    {
        for (const ParametricLine& staticLine : entry.second)
        {
            // Since only position is at play, (and no scaling)
            // We only have to do a simple translation
            ParametricLine collEq;
            collEq.x_0 = staticLine.x_0 - xPos;
            collEq.x_t = staticLine.x_t;
            collEq.y_0 = staticLine.y_0 - yPos;
            collEq.y_t = staticLine.y_t;

            outLines.push_back(collEq);
        }
    }
}

bool CollisionManager::isLitByRadius(vec2 entityPos, const RadiusLightMesh* light) const
{
    vec2 lightPos = light->get_position();

    float distanceX = fmax(0.f, std::fabs(entityPos.x - lightPos.x));
    float distanceY = fmax(0.f, std::fabs(entityPos.y - lightPos.y));
    float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

    if (distance < light->getLightRadius())
    {
        vec2 entityToLight = lightPos - entityPos;
        ParametricLine rayTrace;
        rayTrace.x_0 = 0.f;
        rayTrace.x_t = entityToLight.x;
        rayTrace.y_0 = 0.f;
        rayTrace.y_t = entityToLight.y;

        const ParametricLines blockingEquations = CalculateLightEquations(entityPos.x, entityPos.y, light->getLightRadius());
        for (const ParametricLine& blockingLine : blockingEquations)
        {
            if (LinesCollide(rayTrace, blockingLine))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}