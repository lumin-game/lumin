#pragma once

#include "entity.hpp"

class Glass : public Entity {
public:
	const char* get_texture_path() const override { return textures_path("glass.png"); }
	bool is_player_collidable() const override { return true; }
};
