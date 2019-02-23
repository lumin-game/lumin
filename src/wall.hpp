#pragma once

#include "entity.hpp"

class Wall : public Entity {
public:
	~Wall() override { Entity::destroy(); }

	char* get_texture_path() const override { return textures_path("wall.png"); }
	char* get_lit_texture_path() const override { return nullptr; }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return true; }
	bool is_light_dynamic() const override { return false; }
	EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }
};
