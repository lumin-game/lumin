#pragma once

#include "common.hpp"
#include "level_screen.hpp"


class Screen : public Renderable
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

	bool render_screen();

	void draw_screen();


private:
	bool m_render_level_screen;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
};
