#pragma once

#include "game-screens/game_screen.hpp"

class PauseScreen : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("pause_screen.png"); }
};
