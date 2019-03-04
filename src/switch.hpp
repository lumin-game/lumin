#pragma once

#include "entity.hpp"
#include <iostream>

class Switch : public Entity {
public:
	~Switch() override { Entity::destroy(); }

	char* get_texture_path() const override { return textures_path("switch_off.png"); }
	char* get_lit_texture_path() const override { return textures_path("switch_on.png"); }
	bool is_player_collidable() const override { return false; }
	bool is_light_collidable() const override { return false; }
	bool is_light_dynamic() const override { return true; }
	EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }

	void activate() override;
	void deactivate() override;

	void update(float elapsed_ms);

private:
	bool switchedOn = false;
};
