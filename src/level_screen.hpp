#pragma once

#include "common.hpp"

class LevelScreen : public Renderable
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

	// Renders the water
	void draw(const mat3& projection)override;

	void reset();

private:
	static Texture level_screen_texture;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
};
