#pragma once

#include "entity.hpp"
#include "CollisionManager.hpp"

class Door : public Entity {
public:
    const char* get_texture_path() const override { return textures_path("door_closed.png"); }
    const char* get_lit_texture_path() const override { return textures_path("door_open.png"); }

    // Light up the door when it's activated (likely by a linked switch)
    void activate() override { set_lit(true); }

	bool alwaysRender() { return true; }
	bool activated_by_light() const { return false; }

    bool is_player_inside(Player* player_pos);
	int get_level_index();
	void set_level_index(int);

protected:
	int m_level_index = 1;
};
