#pragma once

#include "common.hpp"
#include "entity.hpp"

class Glass : public Entity {
public:
	~Glass() override { Entity::destroy(); }

	const char* get_texture_path() override { return textures_path("glass.png"); }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return false; }
	EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }
};
