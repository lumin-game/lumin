#pragma once

#include "entity.hpp"

class Fog : public Entity {
public:
    ~Fog() override { Entity::destroy(); }

    char* get_texture_path() const override { return textures_path("fog.png"); }
    bool is_light_collidable() const override { return true; }
};