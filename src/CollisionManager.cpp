#include <memory>
#include <cmath>
#include <iostream>
#include "CollisionManager.hpp"


void CollisionManager::RegisterLight(const LightMesh* light)
{
	if (lightSources.find(light) == lightSources.end())
	{
		lightSources.emplace(light);
	}
}

void CollisionManager::UnregisterLight(const LightMesh* light)
{
	lightSources.erase(light);
}


void CollisionManager::RegisterEntity(const Entity* entity)
{
	if (registeredEntities.find(entity) != registeredEntities.end())
	{
		UnregisterEntity(entity);
	}

	staticLightCollisionLines.emplace(entity, entity->calculate_static_equations());
	registeredEntities.insert(entity);

	if (entity->is_player_collidable())
	{
		staticCollisionEntities.push_back(entity);
	}
}

void CollisionManager::UnregisterEntity(const Entity* entity)
{
	registeredEntities.erase(entity);

	staticLightCollisionLines.erase(entity);
	dynamicLightCollisionLines.erase(entity);
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

const bool CollisionManager::DoorTrace(vec2 bounding_box, vec2 door_pos, float door_width, float door_height, vec2 player_pos) const {
	// Center-to-center distance between two boxes
	float distanceX = std::fabs(door_pos.x - player_pos.x);
	float distanceY = std::fabs(door_pos.y - player_pos.y);

	// Margin is how much distance can be between the two centers before collision
	float xMargin = (bounding_box.x + door_width) / 2;
	float yMargin = (bounding_box.y + door_height) / 2;

	return distanceX < xMargin && distanceY < yMargin;
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
		for (ParametricLine staticLine : entry.second)
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

const std::vector<vec2> CollisionManager::CalculateVertices(float xPos, float yPos, float lightRadius) const
{
	std::vector<vec2> outVertices;
	for (const Entity* entity : registeredEntities)
	{
		CalculateVerticesForEntry(entity, outVertices, xPos, yPos, lightRadius);
	}
	return outVertices;
}

void CollisionManager::CalculateVerticesForEntry(const Entity* entity, std::vector<vec2> &outSet, float xPos, float yPos, float lightRadius) const
{
	const float xDiff = entity->get_position().x - xPos;
	const float yDiff = entity->get_position().y - yPos;
	const float xRadius = entity->get_bounding_box().x / 2;
	const float yRadius = entity->get_bounding_box().y / 2;
	float distanceX = fmax(0.f, std::fabs(xDiff) - xRadius);
	float distanceY = fmax(0.f, std::fabs(yDiff) - yRadius);
	float distanceSqr = distanceX * distanceX + distanceY * distanceY;

	vec2 posToEntity = { xDiff, yDiff };

	if (distanceSqr < ((lightRadius) * lightRadius))
	{
		vec2 topRight = { posToEntity.x + xRadius, posToEntity.y - yRadius };
		vec2 topLeft = { posToEntity.x - xRadius, posToEntity.y - yRadius };
		vec2 bottomRight = { posToEntity.x + xRadius, posToEntity.y + yRadius };
		vec2 bottomLeft = { posToEntity.x - xRadius, posToEntity.y + yRadius };

		outSet.push_back(topRight);
		outSet.push_back(topLeft);
		outSet.push_back(bottomRight);
		outSet.push_back(bottomLeft);
	}
}

bool CollisionManager::IsHitByLight(const vec2 entityPos) const {

	//cycle through all lightsources
	for (auto it = lightSources.begin(); it != lightSources.end(); ++it)
	{
		bool hasCollision = false;
		const LightMesh* light = *it;
		vec2 lightPos = light->get_position();

		float distanceX = fmax(0.f, std::fabs(entityPos.x - lightPos.x));
		float distanceY = fmax(0.f, std::fabs(entityPos.y - lightPos.y));
		//distance from current lightsource to entity in question
		float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

		if (distance < light->getLightRadius() && distance != 0) // check that distance != 0 so that if the position being checked is a light source it can't be lit by itself
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
					hasCollision = true;
					break;
				}
			}

			if (hasCollision == false)
			{
				return true;
			}
		}
	}

	return false;
}

bool CollisionManager::IsHitByLight(const Entity* entity) const {

	vec2 entityPos = entity->get_position();

	for (auto it = lightSources.begin(); it != lightSources.end(); ++it)
	{
		bool hasCollision = false;
		const LightMesh* light = *it;
		vec2 lightPos = light->get_position();

		float distanceX = fmax(0.f, std::fabs(entityPos.x - lightPos.x) - entity->get_bounding_box().x / 2);
		float distanceY = fmax(0.f, std::fabs(entityPos.y - lightPos.y) - entity->get_bounding_box().y / 2);
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
					hasCollision = true;
					break;
				}
			}

			if (hasCollision == false)
			{
				return true;
			}
		}
	}

	return false;
}

vec2 CollisionManager::getClosestVisibleLightSource(const vec2 entityPos) const {

	vec2 closest = { 0,0 };
	float currentClosestDist = 10000000.f;

	for (auto it = lightSources.begin(); it != lightSources.end(); ++it)
	{
		bool hasCollision = false;
		const LightMesh* light = *it;
		vec2 lightPos = light->get_position();

		float distanceX = fmax(0.f, std::fabs(entityPos.x - lightPos.x));
		float distanceY = fmax(0.f, std::fabs(entityPos.y - lightPos.y));
		float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

		if (distance < light->getLightRadius() && distance != 0) // check that distance != 0 so that if the position being checked is a light source it doesn't return itself as the closest light source
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
					hasCollision = true;
					break;
				}
			}

			if (hasCollision == false && distance < currentClosestDist)
			{
				closest = lightPos;
				currentClosestDist = distance;
			}
		}
	}

	return closest;
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

const void CollisionManager::UpdateDynamicLightEquations()
{
	dynamicLightCollisionLines.clear();
	for (const Entity* entity : registeredEntities)
	{
		dynamicLightCollisionLines.emplace(entity, entity->calculate_dynamic_equations());
	}
}