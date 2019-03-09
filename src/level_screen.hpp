#pragma once

#include "render_screen.hpp"

class LevelScreen : public RenderScreen
{
public:
	const char* get_texture_path() const override { return textures_path("level_selection_screen.png"); }
};
