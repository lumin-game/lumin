// Header
#include "world.hpp"
#include "CollisionManager.hpp"
#include "door.hpp"
#include "switch.hpp"

const float NEXT_LEVEL_DELAY = 450.f;
const float SCREEN_SCALE = 1.2f;
#define LASER_UNLOCK 11

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

	m_next_level_elapsed = -1;
	m_save_state = SaveState{};

	m_should_game_start_screen = true;
	m_should_load_level_screen = false;
	m_paused = false;
	m_game_completed = false;
	m_interact = false;
	m_draw_w = false;
	m_show_laser_screen = false;
	m_display_laser_screen_elapsed = 250.f;
	m_screen_size = screen;

	m_load_game_screen.init(screen);
	m_new_game_screen.init(screen);
	m_level_screen.init(screen);
	m_pause_screen.init(screen);
	m_laser_screen.init(screen);
	m_right_top_menu.init(screen);
	m_left_top_menu.init(screen);
	m_current_level_top_menu.init(screen, m_left_top_menu.get_bounding_box());
	m_current_level_top_menu.set_current_level_texture(m_save_state.current_level);
	m_press_w.init(screen);
	m_end_screen.init(screen);

	if (m_save_state.load()) {
		std::cout << "Loaded save state from file.\n" << std::endl;
		m_current_level_top_menu.set_current_level_texture(m_save_state.current_level);
	}

	levelGenerator.create_current_level(m_save_state.current_level, m_player, m_entities);

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
	m_load_game_screen.destroy();
	m_new_game_screen.destroy();
	m_level_screen.destroy();
	m_pause_screen.destroy();
	m_laser_screen.destroy();
	m_right_top_menu.destroy();
	m_left_top_menu.destroy();
	m_current_level_top_menu.destroy();
	m_end_screen.destroy();
	m_press_w.destroy();

	for (int i = 0; i < m_unlocked_level_sparkles.size(); ++i) {
		m_unlocked_level_sparkles[i].destroy();
	}
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms) {
	if (!m_paused) {
		if (m_save_state.current_level == LASER_UNLOCK + 1 && !m_should_game_start_screen) {
			if (m_display_laser_screen_elapsed > 0) {
				m_show_laser_screen = true;
				m_display_laser_screen_elapsed--;
			}
			if (m_display_laser_screen_elapsed == 0) {
				m_show_laser_screen = false;
			}
		} else {
			// reset m_display_laser_screen_elapsed time so that laser splash screen shows up again (after switching levels)
			m_display_laser_screen_elapsed = 250.f;
		}
		// First move the world (entities)
		for (auto entity : m_entities) {
			entity->update(elapsed_ms);
			// If one of our entities is a door, check for player collision
			if (Door* door = dynamic_cast<Door*>(entity)) {
				m_w_position = door->get_position();
				if (door->is_enterable() && door->is_player_inside(&m_player)) {
						if (m_interact) {
							if (m_save_state.skips_allowed < MAX_SKIPS && m_save_state.current_level != door->get_level_index()) {
								m_save_state.skips_allowed++;
							}
							m_save_state.current_level = door->get_level_index();
							next_level();
							m_current_level_top_menu.update(m_save_state.current_level);
							return true;
						}
						else {
							float offset = m_press_w.update();
							m_press_w.set_position({ m_w_position.x, (m_w_position.y + offset) });
							m_draw_w = true;
						}
				}
			}
		}
		for (Entity* entity : m_entities)
		{
			entity->UpdateHitByLight();
		}
		// Then handle light equations
		CollisionManager::GetInstance().UpdateDynamicLightEquations();
		m_player.update(elapsed_ms);

		if (m_save_state.current_level != LASER_UNLOCK + 1) {
			if (m_next_level_elapsed > -1) {
				m_next_level_elapsed += elapsed_ms;
				if (m_next_level_elapsed > NEXT_LEVEL_DELAY) {
					reset_game();
					m_next_level_elapsed = -1;
				}
			}
		}
	}

	m_screen.update(elapsed_ms);

	return true;
}

mat3 World::draw_projection_matrix(int w, int h, float retinaScale, vec2 player_pos){
	float scaled_width = w * SCREEN_SCALE;
	float scaled_height = h * SCREEN_SCALE;

	float left = player_pos.x - scaled_width / retinaScale / 2;
	float top = player_pos.y - scaled_height / retinaScale / 2;
	float right = player_pos.x + scaled_width / retinaScale / 2;
	float bottom = player_pos.y + scaled_height / retinaScale / 2;

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

	for (Entity* entity : m_entities) {
		entity->predraw();
	}

	for (Entity* entity: m_entities) {
		entity->draw(projection_2D);
	}

	m_player.draw(projection_2D);

	float scaled_width = w / SCREEN_SCALE;
	float scaled_height = h / SCREEN_SCALE;
	mat3 menu_projection_2D = draw_projection_matrix(scaled_width, scaled_height, retinaScale, { 0, 0 });

	/////////////////////
	// Truly render to the screen
	if (m_should_game_start_screen) {
		if (m_save_state.data_found) {
			m_load_game_screen.draw(menu_projection_2D);
		} else {
		    m_new_game_screen.draw(menu_projection_2D);
		}
	}
	if (m_show_laser_screen) {
		m_laser_screen.draw(menu_projection_2D);
	}
	if (m_should_load_level_screen) {
		m_level_screen.draw(menu_projection_2D);
		vec2 initial_pos;
		initial_pos.x = m_player.get_position().x - (w / retinaScale / 2) + 320;
		initial_pos.y = m_player.get_position().y - 220;
		float offset = 160;
		int num_col = 5;
		for (int i = 0; i < m_save_state.unlocked_levels; ++i) {
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
		m_draw_w = false;
		m_end_screen.draw(menu_projection_2D);
	}

	if (!m_should_game_start_screen) {
		m_right_top_menu.draw(menu_projection_2D);
		m_left_top_menu.draw(menu_projection_2D);
		m_current_level_top_menu.draw(menu_projection_2D);
	}

	if(m_draw_w){
		m_press_w.draw(projection_2D);
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

void World::reset_game() {
	int w, h;
	glfwGetWindowSize(m_window, &w, &h);

	for (Entity* entity : m_entities) {
		delete entity;
	}
	m_entities.clear();

	m_player.destroy();
	m_press_w.destroy();
	levelGenerator.create_current_level(m_save_state.current_level, m_player, m_entities);
	m_current_level_top_menu.set_current_level_texture(m_save_state.current_level);
	m_player.init();
	m_press_w.init(m_screen_size);

	m_show_laser_screen = false;
	m_should_load_level_screen = false;
	m_show_laser_screen = false;
	m_draw_w = false;

	if (m_save_state.save()) {
		std::cout << "Saved game state to file.\n" << std::endl;
	}
}

void World::load_level_screen(int key_pressed_level) {
	if (m_save_state.current_level == key_pressed_level) {
		m_should_load_level_screen = false;
	} else {
		if (m_save_state.unlocked_levels >= key_pressed_level) {
			m_save_state.current_level = key_pressed_level;
			reset_game();
		} else {
			fprintf(stderr, "Sorry, you need to unlock more levels to switch to this level.");
		}
	}
	m_should_game_start_screen = false;
}

void World::next_level() {
	if (!m_game_completed) {
		if (m_save_state.current_level < MAX_LEVEL) {
			if (m_save_state.current_level != LASER_UNLOCK + 1) {
				m_screen.new_level();
				m_next_level_elapsed = 0.f;
			}
			else {
				reset_game();
			}
		} else if (m_save_state.current_level == MAX_LEVEL) {
			m_game_completed = true;
			return;
		}
		m_save_state.unlocked_levels = std::max(m_save_state.current_level, m_save_state.unlocked_levels);
	}
}

// On key callback
void World::on_key(GLFWwindow* window, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PLAYER MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
			m_player.setJumpPressed(true);
			m_interact = true;
		}
		else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
			m_player.setRightPressed(false);
			m_player.setLeftPressed(true);
		}
		else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
			m_player.setLeftPressed(false);
			m_player.setRightPressed(true);
		}
		// press M key once to load level select screen, press it again to make it disappear unless key buttons(1-5) are selected
		// this can be modified later after incorporating UI buttons
		else if (key == GLFW_KEY_M) {
			// Disable level selection when launch screen is open
			if (!m_should_game_start_screen) {
				m_should_load_level_screen = !m_should_load_level_screen;
				m_load_level = "";
				m_paused = false;
			}
		}
		else if (key == GLFW_KEY_L) {
			m_player.toggleShowPolygon();
		}
		else if (key == GLFW_KEY_P) {
			// Disable level selection when launch screen is open
			if (!m_should_game_start_screen) {
				m_paused = !m_paused;
				m_should_load_level_screen = false;
			}
		}
		else if (m_paused && key == GLFW_KEY_R) {
			m_paused = false;
		}
		else if (!m_paused && key == GLFW_KEY_R) {
			reset_game();
		}
		else if (key == GLFW_KEY_N) {
			if (!m_should_game_start_screen && m_save_state.skips_allowed > 0) {
				m_save_state.current_level += 1;
				m_save_state.skips_allowed--;
				next_level();
			}
		}
	}

	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
			m_player.setJumpPressed(false);
			m_interact = false;
		}
		else if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
			m_player.setLeftPressed(false);
		}
		else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
			m_player.setRightPressed(false);
		}
	}

	if (m_should_load_level_screen) {
		if (key == GLFW_KEY_T) {
			load_level_screen(-1);
		} else {
			if (key == GLFW_KEY_ENTER) {
				load_level_screen(stoi(m_load_level));
			}
			else if (action == GLFW_PRESS) {
				for (int i = GLFW_KEY_0; i < GLFW_KEY_0 + 10; i++) {
					if (key == i) {
						m_load_level += key;
					}
				}
			}
		}
  }

	// Exit Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		// Autosaves the game when user hits ESC
		if (m_save_state.current_level > 0 && m_save_state.save()) {
			std::cout << "Saved game state to file.\n" << std::endl;
		}
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

bool is_button_clicked(double xpos, double ypos, vec2 start_pos, vec2 end_pos) {
	return (xpos > start_pos.x && xpos < end_pos.x && ypos > start_pos.y && ypos < end_pos.y);
}

void World::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	int w, h, ww, hh;
	glfwGetFramebufferSize(m_window, &w, &h);
	glfwGetWindowSize(m_window, &ww, &hh);
	auto retinaScale = (float) (w / ww);
	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// check for clicks on the top menu bar
		vec2 exit_pos_start = {932, 10};
		vec2 exit_pos_end = {970, 25};

		vec2 menu_pos_start = {986, 10};
		vec2 menu_pos_end = {1039, 25};

		vec2 pause_pos_start = {1051, 10};
		vec2 pause_pos_end = {1102, 25};

		vec2 reset_pos_start = {1113, 10};
		vec2 reset_pos_end = {1164, 25};

		if (!m_should_game_start_screen) {
			if (is_button_clicked(xpos, ypos, exit_pos_start, exit_pos_end)) {
				if (m_save_state.current_level > 0 && m_save_state.save()) {
					std::cout << "Saved game state to file.\n" << std::endl;
				}
				destroy();
				exit(0);
			}
			else if (is_button_clicked(xpos, ypos, menu_pos_start, menu_pos_end)) {
				m_should_load_level_screen = !m_should_load_level_screen;
				m_load_level = "";
				m_paused = false;
			}
			else if (is_button_clicked(xpos, ypos, pause_pos_start, pause_pos_end)) {
				m_paused = !m_paused;
				m_should_load_level_screen = false;
			}
			else if (is_button_clicked(xpos, ypos, reset_pos_start, reset_pos_end)) {
				if (m_paused) {
					m_paused = false;
				}
				else {
					reset_game();
				}
			}

			if (m_save_state.current_level == -1 || m_save_state.current_level > LASER_UNLOCK) {
				m_player.setLightMode(!m_player.getLightMode());
			}
		}
		// vec2 initial_pos = { 258, 171 };
		vec2 initial_pos = { 261, 171 };
		int square_length = 122;
		int square_gap = 12;
		int grid_length = square_length + square_gap;
		int num_col = 5;
		int grid_x = 0;
		int grid_y = 0;
		float load_level = 0;
		if (m_should_load_level_screen) {
			if (xpos >= initial_pos.x && xpos <= (float) w / retinaScale - initial_pos.x && ypos >= initial_pos.y && ypos <= (float) h / retinaScale - initial_pos.y) {
				grid_x = floor((xpos - (int)initial_pos.x)/grid_length);
				grid_y = floor((ypos - (int)initial_pos.y)/grid_length);
				if (!(((int) xpos - (int) initial_pos.x) % grid_length >= square_length || ((int) ypos - (int) initial_pos.y) % grid_length >= square_length)) {
					load_level = 1 + grid_x + grid_y * num_col;
					load_level_screen(load_level);
				}
			}
		} else if (m_should_game_start_screen) {
			if (m_save_state.data_found) {
				// if there's already a state saved, start screen will have both "new game" and "load game" options
				vec2 new_pos_start = {300, 567};
				vec2 new_pos_end = {573, 679};

				vec2 load_pos_start = {630, 569};
				vec2 load_pos_end = {902, 676};

				if (is_button_clicked(xpos, ypos, new_pos_start, new_pos_end)) {
					// start new game from level 1
					load_level_screen(1); 
					m_save_state.unlocked_levels = 1;
					m_save_state.skips_allowed = MAX_SKIPS;
				} else if (is_button_clicked(xpos, ypos, load_pos_start, load_pos_end)) {
					// load game from save state
					load_level_screen(m_save_state.current_level);
				}
			} else {
				vec2 new_pos_start = {463, 567};
				vec2 new_pos_end = {738, 678};

				if (is_button_clicked(xpos, ypos, new_pos_start, new_pos_end)) {
					// start new game from level 1
					load_level_screen(1);
					m_save_state.unlocked_levels = 1; 
					m_save_state.skips_allowed = MAX_SKIPS;
				}
			}
		}
	}
}

