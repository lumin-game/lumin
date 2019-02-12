#pragma once

// internal
#include "common.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "screen.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class World
{
public:
	World();
	~World();

	// Creates a window, sets up events and begins the game
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	bool update(float ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over()const;

private:
	bool add_tile(int x_pos, int y_pos, StaticTile tile);

	void create_base_level();

	void print_grid(std::vector<std::vector<char>>& grid);

	void create_level(std::vector<std::vector<char>>& grid);

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow*, int key, int, int action, int mod);

private:
	// Window handle
	GLFWwindow* m_window;

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	// Screen object, we draw everything to another buffer first and then draw the screen using that buffer
	Screen m_screen;

	unsigned int m_points;

	// Game entities
	Player m_player;
	std::vector<Entity*> m_entities;

	float m_current_speed;

	Mix_Music* m_background_music;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1
};
