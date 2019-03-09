#pragma once
#include <vector>
#include <common.hpp>
#include <light_mesh.hpp>
#include "entity.hpp"
#include "firefly.hpp"

class Lantern : public Firefly {

public:
    bool init(float x_pos, float y_pos) override;
    const char* get_texture_path() const override { return textures_path("glass.png"); }
    const char* get_lit_texture_path() const override { return textures_path("glass.png"); }

    void activate() override;

    // Releases all associated resources
    void destroy() override;

    void update(float ms) override;

    void draw(const mat3& projection) override;

private:
    int num_fireflies_drawn = 0;
    float ms_since_activation = 0.f;
};
