#pragma once
#include <vector>
#include <common.hpp>
#include "entity.hpp"

class LightBeam : public Entity {

public:
	const char* get_texture_path() const override {
		return textures_path("glass.png");
	}
	const char* get_lit_texture_path() const override {
		return textures_path("glass.png");
	}

	void setParameters(vec2 dest);

	void update(float ms) override;
	

private:
	vec2 destination;
};
