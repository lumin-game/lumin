// Header
#include "world.hpp"
#include "CollisionManager.hpp"
#include "door.hpp"
#include "switch.hpp"

// stlib

#define MAX_LEVEL 11

// Same as static in c, local to compilation unit
namespace {
	namespace {
		void glfw_err_cb(int error, const char* desc) {
			fprintf(stderr, "%d: %s", error, desc);
		}
	}
}

World::World() {
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

	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);

	GLFWmousebuttonfun mouse_button_func = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_button(wnd, _0, _1, _2); };
	glfwSetMouseButtonCallback(m_window, mouse_button_func);

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	m_current_level = 1;
	// Unlocked levels set to MAX_LEVEL for now for testing purposes
	m_unlocked_levels = MAX_LEVEL;

	m_should_load_level_screen = false;
	m_paused = false;
	m_game_completed = false;
	m_interact = false;

	levelGenerator.create_current_level(m_current_level, m_player, m_entities);
	m_level_screen.init(screen);
	m_pause_screen.init(screen);
	m_right_top_menu.init(screen);
	m_left_top_menu.init(screen);
	m_current_level_top_menu.init(screen);
	m_end_screen.init(screen);

	for (int i = 0; i < MAX_LEVEL; ++i) {
		m_unlocked_level_sparkles.push_back(UnlockedLevelSparkle());
		m_unlocked_level_sparkles[i].init();
	}

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	return m_screen.init();
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);

	Mix_CloseAudio();

	for (Entity* entity : m_entities) {
	    delete entity;
	}
	m_entities.clear();

	m_player.destroy();
	m_screen.destroy();
	m_level_screen.destroy();
	m_pause_screen.destroy();
	m_right_top_menu.destroy();
	m_left_top_menu.destroy();
	m_current_level_top_menu.destroy();
	m_end_screen.destroy();
	for (int i = 0; i < m_unlocked_level_sparkles.size(); ++i) {
		m_unlocked_level_sparkles[i].destroy();
	}
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms) {
	if (!m_paused) {
		// First move the world (entities)
		for (Entity* entity : m_entities)
		{
			entity->UpdateHitByLight();
		}
		for (auto entity : m_entities) {
			entity->update(elapsed_ms);
			// If one of our entities is a door, check for player collision
			if (Door* door = dynamic_cast<Door*>(entity)) {
				if (door->get_lit() && door->is_player_inside(&m_player) && m_interact) {
					m_current_level = door->get_level_index();
					m_current_level_top_menu.update(m_current_level);
					next_level();
					return true;
				}
			}
		}
		// Then handle light equations
		CollisionManager::GetInstance().UpdateDynamicLightEquations();
		m_player.update(elapsed_ms);
	}

	return true;
}

mat3 World::draw_projection_matrix(int w, int h, float retinaScale, vec2 player_pos){
	float left = player_pos.x - (float) w / retinaScale / 2;
	float top = player_pos.y - (float) h / retinaScale / 2;
	float right = player_pos.x + (float) w / retinaScale / 2;
	float bottom = player_pos.y + (float) h / retinaScale / 2;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {{ sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f }};
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

	mat3 projection_2D = draw_projection_matrix(w, h, retinaScale, m_player.get_position());

	m_player.draw(projection_2D);

	for (Entity* entity : m_entities) {
		entity->predraw();
	}

	for (Entity* entity: m_entities) {
		entity->draw(projection_2D);
	}

	m_player.draw(projection_2D);
	mat3 menu_projection_2D = draw_projection_matrix(w, h, retinaScale, { 0, 0 });

	/////////////////////
	// Truly render to the screen
	if (m_should_load_level_screen) {
		m_level_screen.draw(menu_projection_2D);
		vec2 initial_pos;
		initial_pos.x = m_player.get_position().x - (w / retinaScale / 2) + 300;
		initial_pos.y = m_player.get_position().y - 20;
		// Offset is the distance calculated between each level boxes
		float offset = 225;
		// There are 4 boxes per row right now
		int num_col = 4;
		for (int i = 0; i < m_unlocked_levels; ++i) {
			int x = i % num_col;
			int y = i / num_col;
			m_unlocked_level_sparkles[i].set_position(initial_pos, { offset * x, offset * y });
			m_unlocked_level_sparkles[i].draw(projection_2D);
		}
	}
	if (m_paused) {
		m_pause_screen.draw(menu_projection_2D);
	}

	if (m_game_completed) {
		m_end_screen.draw(menu_projection_2D);
	}
	m_right_top_menu.draw(menu_projection_2D);
	m_left_top_menu.draw(menu_projection_2D);
	m_current_level_top_menu.draw(menu_projection_2D);
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

void World::reset_game() {
	int w, h;
	glfwGetWindowSize(m_window, &w, &h);

	for (Entity* entity : m_entities) {
		delete entity;
	}
	m_entities.clear();

	m_player.destroy();
	levelGenerator.create_current_level(m_current_level, m_player, m_entities);
	m_current_level_top_menu.set_current_level_texture(m_current_level);
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

void World::next_level() {
	if (!m_game_completed) {
		if (m_current_level < MAX_LEVEL) {
			reset_game();
		} else if (m_current_level == MAX_LEVEL) {
			m_game_completed = true;
			return;
		}
		m_unlocked_levels = std::max(m_current_level, m_unlocked_levels);
	}
}

// On key callback
void World::on_key(GLFWwindow* window, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PLAYER MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W) {
			m_player.setJumpPressed(true);
			m_interact = true;
		}
		else if (key == GLFW_KEY_A) {
			m_player.setRightPressed(false);
			m_player.setLeftPressed(true);
		}
		else if (key == GLFW_KEY_D) {
			m_player.setLeftPressed(false);
			m_player.setRightPressed(true);
		}
		// press M key once to load level select screen, press it again to make it disappear unless key buttons(1-5) are selected
		// this can be modified later after incorporating UI buttons
		else if (key == GLFW_KEY_M) {
			m_should_load_level_screen = !m_should_load_level_screen;
			m_paused = false;
		}
		else if (key == GLFW_KEY_L) {
			m_player.toggleShowPolygon();
		}
		else if (key == GLFW_KEY_P) {
			m_paused = !m_paused;
			m_should_load_level_screen = false;
		}
		else if (m_paused && key == GLFW_KEY_R) {
			m_paused = false;
		}
		else if (!m_paused && key == GLFW_KEY_R) {
			reset_game();
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			m_player.setJumpPressed(false);
			m_interact = false;
		}
		else if (key == GLFW_KEY_A) {
			m_player.setLeftPressed(false);
		}
		else if (key == GLFW_KEY_D) {
			m_player.setRightPressed(false);
		}
	}

	if (m_should_load_level_screen) {
		if (key == GLFW_KEY_T) {
			load_level_screen(-1);
    }
    else {
		  for (int i = GLFW_KEY_1; i <= GLFW_KEY_1 + MAX_LEVEL; i++){
			if (key == i) {
			  load_level_screen(i - GLFW_KEY_1 + 1);
			}
		  }
	  }
  }

	// Exit Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		destroy();
		exit(0);
	}
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	int w, h, ww, hh;
	glfwGetFramebufferSize(m_window, &w, &h);
	glfwGetWindowSize(m_window, &ww, &hh);
	auto retinaScale = (float) (w / ww);
	float wOffset = (-w / retinaScale) / 2;
	float hOffset = (-h / retinaScale) / 2;

	m_player.setMousePosition({(float) xpos + wOffset, (float) ypos + hOffset});
}

void World::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		m_player.setLightMode(true);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		m_player.setLightMode(false);
	}
}

