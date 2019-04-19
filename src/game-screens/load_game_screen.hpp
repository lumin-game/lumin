#pragma once

#include "game_screen.hpp"

class LoadGameScreen : public GameScreen
{
public:
    const char* get_texture_path() const override { return textures_path("load_game_screen.png"); }
};
