#pragma once

#include "in_game_screen.hpp"

class EndScreen : public InGameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("end_screen.png"); }
};
