#pragma once

#include "entity.hpp"

class Glass : public Entity {
public:
	~Glass() override { Entity::destroy(); }

	char* get_texture_path() const override { return textures_path("wall.png"); }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return false; }
	float get_r() const override { return 0.5; }
	float get_g() const override { return 0.5; }
	float get_b() const override { return 1.0; }
};
