#pragma once

#include "game_screen.hpp"

class RightTopMenu : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("right_top_menu.png"); }
	const vec2 set_translation_scale() override { return { 0.12, 0.12 }; };
	const vec2 set_translation_position() override { return { 3600, -3075 }; };
};
