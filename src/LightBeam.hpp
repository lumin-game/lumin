#pragma once
#include <vector>
#include <common.hpp>
#include "entity.hpp"
#include "switch.hpp"

class LightBeam : public Entity {

public:
	const char* get_texture_path() const override {
		return nullptr;
	}
	const char* get_lit_texture_path() const override {
		return nullptr;
	}

	bool init(float pos_x, float pos_y) override;

	void setParameters(vec2 dest, Switch* parent);

	void update(float ms) override;

	void draw(const mat3& projection) override;

	bool is_destroyed();


private:
	vec2 destination;
	Switch* parent_switch;
	bool destroyed;
	int ms_since_last_particle = 0;
	const int MS_BETWEEN_PARTICLES = 60;
};
