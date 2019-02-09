// Header
#include "world.hpp"
#include "wall.hpp"
#include "CollisionManager.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>

#include <iostream>

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

	m_current_speed = 1.f;

	create_base_level();

	// Maybe not great to pass in 'this'
	// But player (specifically the lightMesh) needs access to static equations
	// Maybe the solution here is a collision manager object or something
	// Or make world a singleton oof
	// TODO: figure out a better way to handle light's dependency on walls
	return m_player.init() && m_screen.init();
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);
	if (m_salmon_dead_sound != nullptr)
		Mix_FreeChunk(m_salmon_dead_sound);
	if (m_salmon_eat_sound != nullptr)
		Mix_FreeChunk(m_salmon_eat_sound);

	Mix_CloseAudio();

	m_player.destroy();
	for (Entity* entity : m_entities) {
		entity->destroy();
	}

	m_entities.clear();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };

	m_player.update(elapsed_ms);

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
	float retinaScale = (float) (w / ww);

	/////////////////////////////////////
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
	float right = (float)w / retinaScale;
	float bottom = (float)h / retinaScale;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	// Drawing entities
	m_player.draw(projection_2D);


	for (Entity* entity: m_entities) {
		entity->draw(projection_2D);
	}

	/////////////////////
	// Truely render to the screen
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

// Creates a new wall and if successfull adds it to the list of wall
bool World::spawn_wall(int x_pos, int y_pos)
{
	Wall *wall = new Wall();
	if (wall->init(x_pos, y_pos)) {
		m_entities.emplace_back(wall);
		return true;
	}
	fprintf(stderr, "Failed to spawn wall");
	return false;
}

void World::create_base_level() {
	// base level is represented by a 0-indexed 10x8 matrix
	std::ifstream in(levels_path("level_data.txt"));
  std::vector<std::vector<char>> grid;

	if(!in) {
		std::cerr << "Cannot open file." << std::endl;
		return;
	}

	std::string row;
	while (std::getline(in, row)) {
    std::string rowS(row.c_str());
    std::vector<char> charVector(rowS.begin(), rowS.end());
    // Dynamically sized vector<char>
    grid.push_back(charVector);
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
	// @ represents walls
  const uint32_t BLOCK_SIZE = 64; // The width and height of walls are 64, so spawning walls at x*BLOCK_SIZE, y*BLOCK_SIZE will align things nicely on a grid
	for (std::size_t i = 0; i < grid.size(); i++) {
		for (std::size_t j = 0; j < grid[i].size(); j++) {
			if (grid[i][j] == '1') {
				spawn_wall(j * BLOCK_SIZE, i * BLOCK_SIZE);
			}
		}
	}
}

// On key callback
void World::on_key(GLFWwindow*, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PLAYER MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_Z) {
			m_player.setZPressed(true);
		}
		else if (key == GLFW_KEY_LEFT) {
			m_player.setLeftPressed(true);
		}
		else if (key == GLFW_KEY_RIGHT) {
			m_player.setRightPressed(true);
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_Z) {
			m_player.setZPressed(false);
		}
		else if (key == GLFW_KEY_LEFT) {
			m_player.setLeftPressed(false);
		}
		else if (key == GLFW_KEY_RIGHT) {
			m_player.setRightPressed(false);
		}
	}

	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		for (Entity* entity : m_entities) {
			delete entity;
		}
		m_entities.clear();
		create_base_level();
		m_player.destroy();
		m_player.init();
		m_current_speed = 1.f;
	}

	// Exit Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		destroy();
		exit(0);
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&  key == GLFW_KEY_COMMA)
		m_current_speed -= 0.1f;
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
		m_current_speed += 0.1f;

	m_current_speed = fmax(0.f, m_current_speed);
}