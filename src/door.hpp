#pragma once

#include "entity.hpp"
#include "CollisionManager.hpp"

class Door : public Entity {
public:
    ~Door() override { Entity::destroy(); }

    char* get_texture_path() const override {
        return textures_path("door_closed.png");
    }

    char* get_lit_texture_path() const override {
        return textures_path("door_open.png");
    }
    bool is_player_collidable() const override { return false; }

    bool is_light_collidable() const override { return false; }

    bool is_light_dynamic() const override { return false; }

    void set_screen_pos(vec2 position) {
        m_screen_pos.x = position.x;
        m_screen_pos.y = position.y - texture->height/2 + 124;
    }

    EntityColor get_color() const override { return EntityColor({1.0, 1.0, 1.0, 1.0}); }

    bool get_player_in(vec2 player_pos);
private:
    bool m_is_player_in;
};
