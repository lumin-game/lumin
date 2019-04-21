#pragma once

#include <map>
#include <set>
#include "entity.hpp"
#include "common.hpp"

// Manages collisions
class CollisionManager
{

public:

struct EntityDistance
{
	Entity* entity;
	float distanceSqr;
};

// CollisionResult is the result of moving one box a set distance.
// Collisions are resolved one by one
struct CollisionResult
{
	float resultXPos = 0.f;					// Final position the box would be
	float resultYPos = 0.f;					// Final position the box would be
	bool topCollision = false;				// If there was on the bottom of the box passed in
	bool bottomCollision = false;			// If there was on the top of the box passed in
	float resultYPush = 0.f;
};

public:
	CollisionManager() {}

	// Singleton
	static CollisionManager& GetInstance()
	{
		static CollisionManager instance;
		return instance;
	}

	// Make sure these functions never get called (or else we may end up with more than 1 CollisionManager)
	CollisionManager(CollisionManager const &) = delete;
	void operator=(CollisionManager const &) = delete;

	// Registers an entity. Should be called on entity init, or to update an entity after it has moved
	void RegisterEntity(Entity* entity);

	// Unregisters an entity. Should be called on destroy.
	void UnregisterEntity(Entity* entity);

	// Registers the player
	void RegisterPlayer(Player* playerPtr);
	void UnregisterPlayer();

	bool CollidesWithPlayer(vec2 boxPosition, vec2 boxBound, vec2 boxDisplacement, CollisionResult& outResult) const;

	void MovePlayer(vec2 movement);

	// Given a box with param dimensions moving a distance of xDist, yDist
	// Return the result of all collisions that will happen
	const CollisionResult BoxTrace(int width, int height, float xPos, float yPos, float xDist, float yDist) const;

	// Check whether door and player are colliding
	// Return true if they are
	bool BoxCollide(vec2 box1Pos, vec2 box1Bound, vec2 box2Pos, vec2 box2Bound) const;

	bool BoxCollideWithPlayer(vec2 boxPos, vec2 boxBound) const;

	// Returns a list of all the vertices of light-blocking objects that are found within a light's radius
	const std::vector<Entity*> GetEntitiesInRange(float xPos, float yPos, float lightRadius) const;

	void UpdateDynamicLightEquations();

	bool LinesCollide(ParametricLine line1, ParametricLine line2) const;
	bool LinesCollide(ParametricLine line1, ParametricLine line2, vec2& collisionPos) const;

	std::set<Entity*> GetEntities() const { return registeredEntities; };

	const RadiusLightMesh* GetPlayerRadiusLightMesh() const;
	const LaserLightMesh* GetPlayerLaserLightMesh() const;

    bool isLitByRadius(vec2 entityPos, const RadiusLightMesh* light) const;

    const ParametricLines CalculateLightEquations(float xPos, float yPos, float lightRadius) const;
    void CalculateLightEquationForEntry(std::pair<const Entity*, ParametricLines> entry, ParametricLines& outLines, float xPos, float yPos, float lightRadius) const;


private:
	std::set<Entity*> registeredEntities;

	// Game entities : Light collision equations
	std::map<const Entity*, const ParametricLines> staticLightCollisionLines;
	std::map<const Entity*, const ParametricLines> dynamicLightCollisionLines;
	
	// Ptr to player, we can keep our position this way. Const as we should never change it.
	Player* player;
};
