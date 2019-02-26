#pragma once

#include "common.hpp"

class PauseScreen : public Renderable
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw(const mat3& projection)override;

  vec2 get_bounding_box()const;

private:
	static Texture pause_screen_texture;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	vec2 m_position;
};