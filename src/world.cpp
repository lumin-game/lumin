// Header
#include "world.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>

#include <iostream>

#if defined(__APPLE__)
 const float RETINA_SCALE = 2.;
#else
 const float RETINA_SCALE = 1.;
#endif

// Same as static in c, local to compilation unit
namespace
{
	namespace
	{
		void glfw_err_cb(int error, const char* desc)
		{
			fprintf(stderr, "%d: %s", error, desc);
		}
	}
}

World::World() :
	m_points(0)
{
	// Seeding rng with random device
	m_rng = std::default_random_engine(std::random_device()());
}

World::~World()
{

}

// World initialization
bool World::init(vec2 screen)
{
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
	return m_player.init(this) && m_screen.init();
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
	for (auto& wall : m_walls)
		wall.destroy();
	m_walls.clear();
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = { (float)w, (float)h };

	// TODO check player - Wall collisions here (but might want to do it after player has moved for the frame)

	// Updating all entities, making the turtle and fish
	// faster based on current
	m_player.update(elapsed_ms);

	for (const auto& wall : m_walls)
	{
		if (m_player.collides_with(wall))
		{
			//do nothing
		}
	}

	return true;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

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
	float right = (float)w / RETINA_SCALE;// *0.5;
	float bottom = (float)h / RETINA_SCALE;// *0.5;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	// Drawing entities
	m_player.draw(projection_2D);


	for (auto& wall: m_walls)
		wall.draw(projection_2D);

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

// Creates a new turtle and if successfull adds it to the list of turtles
bool World::spawn_wall(int x_pos, int y_pos)
{
	Wall wall;
	if (wall.init(x_pos, y_pos))
	{
		m_walls.emplace_back(wall);
		return true;
	}
	fprintf(stderr, "Failed to spawn wall");
	return false;
}

void World::create_base_level() {
	const int BLOCK_SIZE = 64; // The width and height of walls are 64, so spawning walls at x*BLOCK_SIZE, y*BLOCK_SIZE will align things nicely on a grid

	spawn_wall(1*BLOCK_SIZE, 8*BLOCK_SIZE);
	spawn_wall(2 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(3 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(4 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(5 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(6 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(7 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(8 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(9 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(10 * BLOCK_SIZE, 8 * BLOCK_SIZE);
	spawn_wall(10*BLOCK_SIZE, 7 *BLOCK_SIZE);
	spawn_wall(10 * BLOCK_SIZE, 6 * BLOCK_SIZE);
	spawn_wall(8 * BLOCK_SIZE, 4 * BLOCK_SIZE);
	spawn_wall(7 * BLOCK_SIZE, 4 * BLOCK_SIZE);
	spawn_wall(4 * BLOCK_SIZE, 2 * BLOCK_SIZE);
	spawn_wall(3 * BLOCK_SIZE, 2 * BLOCK_SIZE);

	// Calculate parametric equations for edge for each wall
	calculate_static_equations();
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
		for (Wall& wall : m_walls)
		{
			wall.destroy();
		}
		m_walls.clear();
		create_base_level();
		m_player.destroy();
		m_player.init(this);
		m_current_speed = 1.f;
	}

	// Exit Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwDestroyWindow(m_window);
		exit(0);
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&  key == GLFW_KEY_COMMA)
		m_current_speed -= 0.1f;
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
		m_current_speed += 0.1f;

	m_current_speed = fmax(0.f, m_current_speed);
}

void World::calculate_static_equations()
{
	m_staticLightCollisionLines.clear();
	for (Wall& wall : m_walls)
	{
		ParametricLines wallLines = wall.calculate_static_equations();
		m_staticLightCollisionLines.insert(m_staticLightCollisionLines.end(), wallLines.begin(), wallLines.end());
	}
}
