#pragma once

#include "game_screen.hpp"
#include <ctime>

class PressW : public GameScreen
{
public:
	float update();
	const char* get_texture_path() const override { return textures_path("press_w.png"); };
	void set_translation_scale() override { m_scale = { 0.1, 0.1 }; };
	void set_translation_position(vec2 screen, bool is_left) override {};
};
