#pragma once

#include "game_screen.hpp"

class LaserScreen : public GameScreen
{
public:
	const char* get_texture_path() const override { return textures_path("laser_screen.png"); }
  void display_screen(float ms);
};
