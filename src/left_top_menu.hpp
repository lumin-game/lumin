#pragma once

#include "game_screen.hpp"

class LeftTopMenu : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("left_top_menu.png"); }
	const vec2 set_translation_scale() override { return { 0.1, 0.1 }; };
	const vec2 set_translation_position() override { return { -5168, -3700 }; };
};
