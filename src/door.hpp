#pragma once

#include "entity.hpp"
#include "CollisionManager.hpp"

class Door : public Entity {
public:
    const char* get_texture_path() const override { return textures_path("door_closed.png"); }
    const char* get_lit_texture_path() const override { return textures_path("door_open.png"); }

    bool init(float x_pos, float y_pos) override;

    // Light up the door when it's activated (likely by a linked switch)
    void activate() override { set_lit(true); }
    void deactivate() override { set_lit(false); }

	bool alwaysRender() override { return true; }
	bool activated_by_light() const override { return false; }

    bool is_player_inside(Player* player_pos);
	int get_level_index();
	void set_level_index(int);

protected:
	int m_level_index = 1;
};
