// Header
#include "world.hpp"
#include "wall.hpp"
#include "glass.hpp"
#include "fog.hpp"
#include "switch.hpp"
#include "movable_wall.hpp"
#include "CollisionManager.hpp"
#include "firefly.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>

#include <iostream>

#define BLOCK_SIZE 64

// Same as static in c, local to compilation unit
namespace {
	namespace {
		void glfw_err_cb(int error, const char* desc) {
			fprintf(stderr, "%d: %s", error, desc);
		}
	}
}

World::World() : m_points(0) {
	// Seeding rng with random device
	m_rng = std::default_random_engine(std::random_device()());
}

World::~World() {

}

// World initialization
bool World::init(vec2 screen) {
	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization
	// Core Opengl 3.
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Lumin", nullptr, nullptr);
	if (m_window == nullptr)
		return false;

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	gl3w_init();

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(m_window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((World*)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3); };
	glfwSetKeyCallback(m_window, key_redirect);

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	m_current_level = 1;

	m_unlocked_levels = 5;

	m_max_level = 5;

	m_should_load_level_screen = false;
	m_paused = false;

	create_current_level();
	m_level_screen.init();
	m_pause_screen.init();

	// Maybe not great to pass in 'this'
	// But player (specifically the lightMesh) needs access to static equations
	// Maybe the solution here is a collision manager object or something
	// Or make world a singleton oof
	// TODO: figure out a better way to handle light's dependency on walls
	return m_screen.init();
}

void World::create_firefly(vec2 pos)
{
	Firefly* firefly = new Firefly();
	firefly->init();
	firefly->set_position(pos);
	m_fireflies.push_back(firefly);
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);

	Mix_CloseAudio();

	m_player.destroy();
	for (Entity* entity : m_entities) {
	    delete entity;
	}
	m_entities.clear();

	for (Firefly* firefly : m_fireflies) {
		delete firefly;
	}
	m_fireflies.clear();
	m_screen.destroy();
	m_level_screen.destroy();
	m_pause_screen.destroy();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms) {
	if (!m_paused) {
		// First move the world (entities)
		for (auto entity : m_entities) {
			entity->update(elapsed_ms);

			// If one of our entities is a door, check for player collision
			if (Door* door = dynamic_cast<Door*>(entity)) {
			    if (door->get_lit() && door->is_player_inside(&m_player)) {
			    	update_level();
			    	return true;
			    }
			}
		}

		// Then handle light equations
		CollisionManager::GetInstance().UpdateDynamicLightEquations();

		m_player.update(elapsed_ms);

		for (Firefly* firefly : m_fireflies) {
			firefly->update(elapsed_ms);
		}
	}

	return true;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw() {
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	// Check for discrepancy between window/frame buffer (high DPI display)
	int ww, hh;
	glfwGetWindowSize(m_window, &ww, &hh);
	auto retinaScale = (float) (w / ww);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0.f, 0.f, 0.f, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fake projection matrix, scales with respect to window coordinates
	// PS: 1.f / w in [1][1] is correct.. do you know why ? (:
	float left = 0.f;// *-0.5;
	float top = 0.f;// (float)h * -0.5;
	float right = (float) w / retinaScale;
	float bottom = (float)h / retinaScale;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };
	// Drawing entities
	m_player.calculate_screen_pos(ww, hh);

	for (Entity* entity: m_entities) {
		float screen_pos_x = entity->get_position().x - m_player.get_position().x + m_player.get_screen_pos().x;
		float screen_pos_y = entity->get_position().y - m_player.get_position().y + m_player.get_screen_pos().y;
		vec2 screen_pos = {screen_pos_x, screen_pos_y};
		entity->set_screen_pos(screen_pos);
		entity->draw(projection_2D);
	}

	for (Firefly* firefly : m_fireflies) {
		float screen_pos_x = firefly->get_position().x - m_player.get_position().x + m_player.get_screen_pos().x;
		float screen_pos_y = firefly->get_position().y - m_player.get_position().y + m_player.get_screen_pos().y;
		vec2 screen_pos = { screen_pos_x, screen_pos_y };
		firefly->set_screen_pos(screen_pos);
		firefly->draw(projection_2D);
	}

	m_player.draw(projection_2D);

	/////////////////////
	// Truely render to the screen
	if (m_should_load_level_screen) {
		m_level_screen.draw(projection_2D);
	}
	if (m_paused) {
		m_pause_screen.draw(projection_2D);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screen_tex.id);

	m_screen.draw(projection_2D);

	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over()const
{
	return glfwWindowShouldClose(m_window);
}

bool World::add_tile(int x_pos, int y_pos, StaticTile tile) {
	Entity *level_entity = nullptr;

	switch (tile) {
		case WALL:
			level_entity = new Wall();
			break;
		case GLASS:
			level_entity = new Glass();
			break;
		case DARKWALL:
			// TODO: add dark wall entity
			break;
		case LIGHTWALL:
			// TODO: add light wall entity
			break;
		case FOG:
			level_entity = new Fog();
			break;
		case FIREFLY:
			create_firefly({ (float) x_pos * BLOCK_SIZE, (float) y_pos * BLOCK_SIZE });
			return true;
		case PLAYER:
			m_player.init();
			// spawn player 1 tile higher to ensure that the player doesn't fall
		 	m_player.setPlayerPosition({ (float) x_pos * BLOCK_SIZE, (float) (y_pos - 1) * BLOCK_SIZE });
		 	return true;
	}

	if (!level_entity) {
		fprintf(stderr, "Level entity is not set \n");
		return false;
	}
	if (level_entity->init(x_pos * BLOCK_SIZE, y_pos * BLOCK_SIZE)) {
		m_entities.push_back(level_entity);
		return true;
	}
	fprintf(stderr, "Failed to add %u tile \n", tile);
	return false;
}

void World::create_current_level() {
	std::ifstream in(levels_path("level_" + std::to_string(m_current_level) + ".txt"));

	if(!in) {
		std::cerr << "Cannot open file. \n" << std::endl;
		return;
	}

	std::vector<std::vector<char>> grid;
	std::map<char, std::pair<int, int>> dynamicEntityLocs;
	std::map<char, Entity*> dynamicEntities;

	std::string row;
	int y = 0;

	while (std::getline(in, row)) {
		std::vector<char> charVector(row.begin(), row.end());

		// Ignore empty lines in the level file
		if (!charVector.empty()) {

			if (charVector[0] == '?') {
				// Parse entity declaration
				const char name = charVector[1];
				const char type = charVector[2];

				Entity* entity;

				switch (type) {
					// Switch
					case '/':
					    entity = new Switch();
						break;

					// Moving platform
					case '_':
					    entity = new MovableWall();
					    // TODO: set properties of movable wall dynamically in level file
						((MovableWall*) entity)->set_movement_properties(0.f, -3.f, 0.2, false, false);
						break;

					case '|':
						entity = new Door();

						// Make default state of door open; if we later link it to a switch,
						// we turn its default state to off as part of the linking process.
						entity->set_lit(true);
						break;

					default:
						fprintf(stderr, "Unknown entity declaration in level file: %c: %c\n", name, type);
						continue;
				}

				std::pair<int, int> coord = dynamicEntityLocs.find(name)->second;
				entity->init(coord.first * BLOCK_SIZE, coord.second * BLOCK_SIZE);
				dynamicEntities.insert(std::pair<char, Entity*>(name, entity));
				m_entities.push_back(entity);

			} else if (charVector[0] == '=') {
				// Parse entity relationship
				if (dynamicEntities.find(charVector[1]) == dynamicEntities.end()) {
					fprintf(stderr, "Couldn't parse first entity in relationship: %c\n", charVector[1]);
					continue;
				}

				Entity* entity_1 = dynamicEntities.find(charVector[1])->second;

				if (dynamicEntities.find(charVector[2]) == dynamicEntities.end()) {
					fprintf(stderr, "Couldn't parse second entity in relationship: %c\n", charVector[2]);
					continue;
				}

				Entity* entity_2 = dynamicEntities.find(charVector[2])->second;

				if (!entity_1 || !entity_2) {
					continue;
				}

				entity_1->register_entity(entity_2);

				// Door logic!
				if (Door* door = dynamic_cast<Door*>(entity_2)) {
				    door->set_lit(false);
				}

			} else {
				// Keep track of dynamic dynamicEntities
				for (int x = 0; x < charVector.size(); x++) {
					const char c = charVector[x];
					if (('0' <= c && c <= '9') || ('A' <= c && c <= 'Z')) {
						const std::pair<int, int> coord = std::make_pair(x, y);
						dynamicEntityLocs.insert(std::pair<char, std::pair<int, int>>(c, coord));
					}
				}

				// Push entire row into grid vector
				grid.push_back(charVector);
				y++;
			}
		}
	}

	in.close();
	create_level(grid);
}

// Just to print the grid (testing purposes)
void World::print_grid(std::vector<std::vector<char>>& grid) {
	for (std::vector<char> row : grid) {
    for (char cell : row) {
      std::cout << cell << " ";
    }
    std::cout << std::endl;
  }
}

void World::create_level(std::vector<std::vector<char>>& grid) {
	std::map<char, StaticTile> tile_map = {
		{'#', WALL},
		{'$', GLASS},
		{'+', DARKWALL},
		{'-', LIGHTWALL},
		{'~', FOG},
		{'*', FIREFLY},
		{'&', PLAYER}
	};

	for (int y = 0; y < grid.size(); y++) {
		for (int x = 0; x < grid[y].size(); x++) {
			auto tile = tile_map.find(grid[y][x]);
			if (tile != tile_map.end()) {
				add_tile(x, y, tile->second);
			}
		}
	}
}

void World::reset_game() {
	int w, h;
	glfwGetWindowSize(m_window, &w, &h);
	for (Entity* entity : m_entities) {
		delete entity;
	}
	m_entities.clear();
	for (Firefly* firefly : m_fireflies) {
		delete firefly;
	}
	m_fireflies.clear();
	m_player.destroy();
	create_current_level();
	m_player.init();
	m_should_load_level_screen = false;
}

void World::load_level_screen(int key_pressed_level) {
	if (m_current_level == key_pressed_level) {
		m_should_load_level_screen = false;
	} else {
		if (m_unlocked_levels >= key_pressed_level) {
			m_current_level = key_pressed_level;
			reset_game();
		} else {
			fprintf(stderr, "Sorry, you need to unlock more levels to switch to this level.");
		}
	}
}

void World::update_level() {
	if (m_current_level < m_max_level) {
		m_current_level++;
		reset_game();
	} else if (m_current_level == m_max_level){
		// TODO: Maybe project a screen displaying that user has completed all levels?
		fprintf(stderr, "Congratulations! You've conquered all levels in the game!");
	}
	m_unlocked_levels = std::max(m_current_level, m_unlocked_levels);
}

// On key callback
void World::on_key(GLFWwindow* window, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PLAYER MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_Z || key == GLFW_KEY_UP) {
			m_player.setZPressed(true);
		}
		else if (key == GLFW_KEY_LEFT) {
			m_player.setLeftPressed(true);
		}
		else if (key == GLFW_KEY_RIGHT) {
			m_player.setRightPressed(true);
		}
		// press M key once to load level select screen, press it again to make it disappear unless key buttons(1-5) are selected
		// this can be modified later after incorporating UI buttons
		else if (key == GLFW_KEY_M) {
			m_should_load_level_screen = !m_should_load_level_screen;
		}
		else if (key == GLFW_KEY_P) {
			m_paused = !m_paused;
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_Z || key == GLFW_KEY_UP) {
			m_player.setZPressed(false);
		}
		else if (key == GLFW_KEY_LEFT) {
			m_player.setLeftPressed(false);
		}
		else if (key == GLFW_KEY_RIGHT) {
			m_player.setRightPressed(false);
		}
	}

	if (m_should_load_level_screen) {
		if (key == GLFW_KEY_1) {
			load_level_screen(1);
		} else if (key == GLFW_KEY_2) {
			load_level_screen(2);
		} else if (key == GLFW_KEY_3) {
			load_level_screen(3);
		} else if (key == GLFW_KEY_4) {
			load_level_screen(4);
		} else if (key == GLFW_KEY_5) {
			load_level_screen(5);
		}
	}

	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		reset_game();
	}

	// Exit Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		destroy();
		exit(0);
	}
}
