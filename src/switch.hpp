#pragma once

#include "entity.hpp"
#include "LightBeamParticle.hpp"
#include <iostream>

#include <SDL.h>
#include <SDL_mixer.h>

class Switch : public Entity {
public:
	const char* get_texture_path() const override {
		return mToggleSwitch ? textures_path("button_off.png") : textures_path("switch_off.png");
	}
	const char* get_lit_texture_path() const override {
		return mToggleSwitch ? textures_path("button_on.png") : textures_path("switch_on.png");
	}
	const char* get_audio_path() const override { return audio_path("switch_sound.wav"); }
	bool is_light_dynamic() const override { return true; }

	bool init(float x_pos, float y_pos) override;

	void activate() override;
	void deactivate() override;

	void update(float ms) override;
	void draw(const mat3& projection) override;

	void set_toggle_switch(bool isToggle);

	void add_beam_particle(LightBeamParticle* particle);

private:
	bool mToggleSwitch = false;
	std::set<Entity*> light_beams;
	std::set<LightBeamParticle*> light_beam_particles;
	Texture particle_texture;
};
