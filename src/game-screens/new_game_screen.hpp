#pragma once

#include "game_screen.hpp"

class NewGameScreen : public GameScreen
{
public:
    const char* get_texture_path() const override { return textures_path("new_game_screen.png"); }
};
