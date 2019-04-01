#pragma once

#include "common.hpp"
#include "level_screen.hpp"


class Screen : public Renderable
{
public:
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	void new_level();

	void update(float elapsed_ms);

	// Renders the water
	void draw(const mat3& projection)override;

	bool render_screen();

	void draw_screen();


private:
	bool m_render_level_screen;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	float m_new_level_elapsed;
	float m_new_level_fade;
};
