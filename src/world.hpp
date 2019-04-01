#pragma once

// internal
#include "common.hpp"
#include "player.hpp"
#include "entity.hpp"
#include "screen.hpp"
#include "level_screen.hpp"
#include "pause_screen.hpp"
#include "end_screen.hpp"
#include "level_unlocked_sparkle.hpp"
#include "right_top_menu.hpp"
#include "left_top_menu.hpp"
#include "current_level.hpp"
#include "LevelGenerator.hpp"
#include "press_w.hpp"

// stlib
#include <vector>
#include <random>
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <ctime>
#include <string>

#define MAX_LEVEL 14

struct SaveState {
    int current_level = 1;
    int unlocked_levels = MAX_LEVEL;

    bool save() {
    	std::ofstream file;
    	file.open("lumin.sav");
    	file << std::to_string(current_level) << "\n" << std::to_string(unlocked_levels);
    	file.close();
    	return true;
    }

    bool load() {
		std::ifstream in("lumin.sav");
		if (!in) {
			std::cerr << "Cannot open file. \n" << std::endl;
			return false;
		}

		std::string line;
		int i = 0;
		while (std::getline(in, line)) {
			switch (i) {
				case 0:
					current_level = std::stoi(line);
					break;
				case 1:
					unlocked_levels = std::stoi(line);
					break;
				default:
					break;
			}

			i += 1;
		}

		return true;
    }
};

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

	// Draws a projection matrix
	mat3 draw_projection_matrix(int w, int h, float retinaScale, vec2 player_pos);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over()const;

private:
	void reset_game();

	void load_level_screen(int key_pressed_level);

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow*, int key, int, int action, int mod);
	void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);

	void next_level();

private:
	std::string m_load_level;
	// Window handle
	GLFWwindow* m_window;

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;

	LevelGenerator levelGenerator;

	// Screen object, we draw everything to another buffer first and then draw the screen using that buffer
	Screen m_screen;
	EndScreen m_end_screen;
	LevelScreen m_level_screen;
	PauseScreen m_pause_screen;
	std::vector<UnlockedLevelSparkle> m_unlocked_level_sparkles;
	LeftTopMenu m_left_top_menu;
	RightTopMenu m_right_top_menu;
	CurrentLevel m_current_level_top_menu;
	PressW m_press_w;

	float m_next_level_elapsed;
	SaveState m_save_state;

	// Game entities
	Player m_player;
	std::vector<Entity*> m_entities;
	Mix_Music* m_background_music;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1

	bool m_should_load_level_screen;
	bool m_paused;
	bool m_game_completed;
	bool m_interact;
	bool m_draw_w;

	vec2 m_w_position;
	vec2 m_screen_size;
};
