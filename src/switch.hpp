#pragma once

#include "entity.hpp"
#include <iostream>

class Switch : public Entity {
public:
	~Switch() override { Entity::destroy(); }

	char* get_texture_path() const override {
		std::cout << m_is_lit << std::endl;
		if (m_is_lit) {
			return textures_path("switch_on.png");
		} else {
			return textures_path("switch_off.png");
		}
	}
	bool is_player_collidable() const override { return false; }
	bool is_light_collidable() const override { return false; }
	bool is_light_dynamic() const override { return true; }
	EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }
	void set_lit(bool lit) override { m_is_lit = lit; }
private:
	bool m_is_lit;
};
