#pragma once

#include "entity.hpp"

class Glass : public Entity {
public:
	~Glass() override { Entity::destroy(); }

	const char* get_texture_path() const override { return textures_path("glass.png"); }
	bool is_player_collidable() const override { return true; }
};
