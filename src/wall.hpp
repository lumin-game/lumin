#pragma once

#include "entity.hpp"

class Wall : public Entity {
public:
	const char* get_texture_path() const override { return textures_path("wall.png"); }
	const char* get_lit_texture_path() const override { return textures_path("wall.png"); }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return true; }
};
