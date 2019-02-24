#pragma once

#include "common.hpp"


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

	bool render_menu();

	bool render_screen();

	void draw_menu(const mat3& projection);

	void draw_screen();

	bool get_render_menu();

	void set_render_menu(bool m_is_m_pressed);

	void reset();

private:
	static Texture menu_texture;
	bool m_render_menu;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	ParentData m_parent;
};
