#pragma once

#include "in_game_screen.hpp"

class PauseScreen : public InGameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("pause_screen.png"); }
};