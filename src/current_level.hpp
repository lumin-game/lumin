#pragma once

#include "common.hpp"

class CurrentLevel : public Renderable
{
public:
	bool init(vec2 screen, vec2 offset);

	void destroy();

	int get_current_level();

	void set_current_level_texture(int current_level);

	void draw(const mat3& projection)override;

	void update(int current_level);

	void set_position(vec2 position, vec2 offset = {0, 0});

  vec2 get_bounding_box()const;


private:
  TexturedVertex vertices[4];
	static Texture current_level_texture;
	// TODO: modify this value once we know the total number of levels
	static const int TOTAL_LEVELS = 15;
	vec2 m_scale = { 0.11, 0.11 };
	vec2 m_position;
	int m_current_level;
};
