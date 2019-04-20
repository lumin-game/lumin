#pragma once

#include "common.hpp"

class World;

class LaserLightMesh : public Renderable
{
public:
	struct ParentData
	{
		vec2 m_position;
		vec2 m_mousePosition;
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

	void toggleShowPolygon() {
		m_enablePolygon = !m_enablePolygon;
	};

    // how long the laser actually is
    float actualLength;

    // Angle the laser is facing
    float lightAngle;

private:
	// Recreate polygonial mesh based on objects that block light around us. Happens per frame.
	int UpdateVertices();

	ParametricLines ConvertLinesToAngle(ParametricLines lines, float cosA, float sinA);

	// Data from the parent object (only player for now, but maybe lanterns too in future)
	ParentData m_parent;

	// how long
	float m_laserLength;

	float m_laserWidth;

	bool m_enablePolygon = false;
};
