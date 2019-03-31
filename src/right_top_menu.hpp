#pragma once

#include "game_screen.hpp"

class RightTopMenu : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("right_top_menu.png"); }
	void set_translation_scale() override { m_scale = { 0.12, 0.12 }; };
	void set_translation_position() override { m_position = { 432, -369 }; };
};

