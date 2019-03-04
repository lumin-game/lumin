#pragma once

#include "entity.hpp"

class Glass : public Entity {
public:
	~Glass() override { Entity::destroy(); }

	char* get_texture_path() const override { return textures_path("glass.png"); }
	char* get_lit_texture_path() const override { return nullptr; }
	bool is_player_collidable() const override { return true; }
	bool is_light_collidable() const override { return false; }
	bool is_light_dynamic() const override { return false; }

	void activate() override {};
	void deactivate() override {};

	EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }
};
