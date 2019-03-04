#pragma once

#include "entity.hpp"
#include "CollisionManager.hpp"

class Door : public Entity {
public:
    ~Door() override { Entity::destroy(); }

    const char* get_texture_path() const override { return textures_path("door_closed.png"); }
    const char* get_lit_texture_path() const override { return textures_path("door_open.png"); }

    void activate() override { set_lit(true); };

    // Offset door texture to align with ground
    void set_screen_pos(vec2 position) {
        m_screen_pos.x = position.x;
        m_screen_pos.y = position.y - texture->height/2 + 124;
    }

    bool get_player_in(vec2 player_pos);
private:
    bool m_is_player_in;
};
