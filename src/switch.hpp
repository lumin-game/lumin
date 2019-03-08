#pragma once

#include "entity.hpp"
#include <iostream>

#include <SDL.h>
#include <SDL_mixer.h>

class Switch : public Entity {
public:
	const char* get_texture_path() const override { return textures_path("switch_off.png"); }
	const char* get_lit_texture_path() const override { return textures_path("switch_on.png"); }
	const char* get_audio_path() const { return audio_path("switch_on.wav"); }
	bool is_light_dynamic() const override { return true; }

	void activate() override;
	void deactivate() override;
};
