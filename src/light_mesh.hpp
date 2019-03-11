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
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the player
	void draw(const mat3& projection) override;

	void SetParentData(ParentData data) { m_parent = data; }

	vec2 get_position() const;

	float getLightRadius() const;

	void toggleShowPolygon() {
		m_enablePolygon = !m_enablePolygon;
	};

private:
	// Recreate polygonial mesh based on objects that block light around us. Happens per frame.
	int UpdateVertices();

	// Data from the parent object (only player for now, but maybe lanterns too in future)
	ParentData m_parent;

	// how larj
	float m_lightRadius;

	bool m_enablePolygon = false;
};
