#pragma once

#include "in_game_screen.hpp"

class LevelScreen : public InGameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("level_selection_screen.png"); }
};
