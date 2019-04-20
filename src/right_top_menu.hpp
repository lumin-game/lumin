#pragma once

#include "game-screens/game_screen.hpp"

class RightTopMenu : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("right_top_menu.png"); }
	void set_translation_scale() override { m_scale = { 0.12, 0.12 }; };
	void set_translation_position(vec2 screen, bool is_true) override { 
		is_true = false; 
		m_position = calculate_position(screen, is_true); 
	};

};

