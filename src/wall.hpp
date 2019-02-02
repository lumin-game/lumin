#pragma once

#include "entity.hpp"

class Wall : public Entity {
public:
	~Wall() override { Entity::destroy(); }

	const char* get_texture_path() override { return textures_path("wall.png"); }
	bool is_player_collidable() override { return true; }
	bool is_light_collidable() override { return true; }
};
