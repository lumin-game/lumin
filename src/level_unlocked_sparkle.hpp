#pragma once

#include "common.hpp"

class UnlockedLevelSparkle : public Renderable
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw(const mat3& projection)override;

	// Default screen position doesn't have an offset value
	void set_position(vec2 position, vec2 offset = {0, 0});

  vec2 get_bounding_box()const;


private:
  TexturedVertex vertices[4];
	static Texture unlocked_level_sparkle_texture;
	vec2 m_scale; // 1.f in each dimension. 1.f is as big as the associated texture
	vec2 m_position;
};
