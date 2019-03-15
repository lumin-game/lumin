#pragma once

#include "game_screen.hpp"

class EndScreen : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("end_screen.png"); }
};
