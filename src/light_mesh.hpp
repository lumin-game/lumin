#pragma once

#include "common.hpp"

class World;

class LightMesh : public Renderable
{
public:
	struct ParentData
	{
		vec2 m_position;
		vec2 m_screen_pos;
	};

public:
	// Creates all the associated render resources and default transform
	bool init(const World* world);

	// Releases all associated resources
	void destroy();

	// Renders the player
	void draw(const mat3& projection) override;

	void SetParentData(ParentData data) { m_parent = data; }

private:
	// Data from the parent object (only player for now, but maybe lanterns too in future)
	ParentData m_parent;

	// Collision equations for the static walls (not updated per frame)
	const ParametricLines* m_staticCollisionPtr;

	// Collision equations in own own coordinate system (updated per frame for the player)
	ParametricLines m_collisionEquations;

	// how larj
	float m_lightRadius;
};
