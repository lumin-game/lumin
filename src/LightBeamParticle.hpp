#pragma once
#include <vector>
#include <common.hpp>
#include "entity.hpp"

class LightBeamParticle : public Entity {

public:
	const char* get_texture_path() const override {
		return textures_path("glass.png");
	}
	const char* get_lit_texture_path() const override {
		return textures_path("glass.png");
	}

	void update(float ms) override;

	void draw(const mat3& projection) override;

	bool is_destroyed();

private:
	vec2 destination;
	float m_opacity = 1.0f;
	bool destroyed = false;
};
