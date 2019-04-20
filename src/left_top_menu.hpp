#pragma once

#include "game-screens/game_screen.hpp"

class LeftTopMenu : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("left_top_menu.png"); }
	void set_translation_scale() override { m_scale = { 0.1, 0.1 }; };
};
