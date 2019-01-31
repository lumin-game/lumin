#pragma once

#include "common.hpp"

class PlayerMesh : public Renderable
{
public:
	struct ParentData
	{
		vec2 m_position;
	};

private:
	static Texture player_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the player
	void draw(const mat3& projection) override;

	void turn_right();

	void turn_left();

	void SetParentData(ParentData data) { m_parent = data; }

	int GetPlayerWidth() const;

	int GetPlayerHeight() const;

private:
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
};
