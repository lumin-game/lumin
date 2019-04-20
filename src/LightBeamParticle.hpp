#pragma once
#include <vector>
#include <common.hpp>
#include "entity.hpp"

class LightBeamParticle : public Entity {

public:
	const char* get_texture_path() const override {
		return nullptr;
	}
	const char* get_lit_texture_path() const override {
		return nullptr;
	}

	void set_texture(Texture* tex);

	void update(float ms) override;

	bool init(float x_pos, float y_pos) override;

	void draw(const mat3& projection) override;

	bool is_destroyed();

private:
	vec2 destination;
	float m_opacity = 0.8f;
	bool destroyed = false;
};
