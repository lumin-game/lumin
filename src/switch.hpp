#pragma once

#include "entity.hpp"
#include "movable_wall.hpp"
#include "door.hpp"
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

	void register_movable_wall(MovableWall* wall);
	void register_door(Door* door);

	void trigger_switch_on();
	void trigger_switch_off();

	void update();

private:
	std::vector<MovableWall*> m_movableWalls;
	Door* m_door;
	bool switchedOn = false;
};
