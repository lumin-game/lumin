#pragma once

#include "render_screen.hpp"

class PauseScreen : public RenderScreen
{
public:
	const char* get_texture_path() const override { return textures_path("pause_screen.png"); }
};