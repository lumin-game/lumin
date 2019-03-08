#pragma once

// internal
#include "common.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "screen.hpp"
#include "level_screen.hpp"
#include "pause_screen.hpp"
#include "level_unlocked_sparkle.hpp"
#include "LevelGenerator.hpp"

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
	void reset_game();

	void load_level_screen(int key_pressed_level);

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow*, int key, int, int action, int mod);

	void next_level();

private:
	// Window handle
	GLFWwindow* m_window;

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	LevelGenerator levelGenerator;

	// Screen object, we draw everything to another buffer first and then draw the screen using that buffer
	Screen m_screen;
	LevelScreen m_level_screen;
	PauseScreen m_pause_screen;
	std::vector<UnlockedLevelSparkle> m_unlocked_level_sparkles;

	int m_current_level;

	// number of levels that user has unlocked
	int m_unlocked_levels;

	// Game entities
	Player m_player;
	std::vector<Entity*> m_entities;
	Mix_Music* m_background_music;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1

	bool m_should_load_level_screen;
	bool m_paused;
};
