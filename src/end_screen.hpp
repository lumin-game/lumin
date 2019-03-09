#pragma once

#include "render_screen.hpp"

class EndScreen : public RenderScreen
{
public:
	const char* get_texture_path() const override { return textures_path("end_screen.png"); }
};
