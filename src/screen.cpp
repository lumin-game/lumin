#include "screen.hpp"

#include <iostream>
#include <algorithm>

const float MAX_FADE_TIME = 1000.f;

bool Screen::init() {
	m_new_level_elapsed = -1;
	m_new_level_fade = -1;
  	return render_screen();
}

void Screen::destroy() {
	glDeleteBuffers(1, &mesh.vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Screen::new_level() {
	m_new_level_elapsed = 0.f;
}

void Screen::update(float elapsed_ms) {
	if (m_new_level_elapsed != -1) {
		m_new_level_elapsed += elapsed_ms;
        m_new_level_fade = m_new_level_elapsed;
		if (m_new_level_elapsed < MAX_FADE_TIME) {
			m_new_level_fade = std::min(m_new_level_fade, MAX_FADE_TIME - m_new_level_fade);
		} else {
			m_new_level_elapsed = -1;
			m_new_level_fade = -1;
		}
	}
}

void Screen::draw(const mat3& projection) {
  return draw_screen();
}


bool Screen::render_screen(){
	static const GLfloat screen_vertex_buffer_data[] = {
		-1.05f, -1.05f, 0.0f,
		1.05f, -1.05f, 0.0f,
		-1.05f,  1.05f, 0.0f,
		-1.05f,  1.05f, 0.0f,
		1.05f, -1.05f, 0.0f,
		1.05f,  1.05f, 0.0f,
	};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertex_buffer_data), screen_vertex_buffer_data, GL_STATIC_DRAW);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("screen.vs.glsl"), shader_path("screen.fs.glsl")))
		return false;
	return true;
}

void Screen::draw_screen(){
	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Setting shaders
	glUseProgram(effect.program);

	// Set screen_texture sampling to texture unit 0
	// Set clock
	GLuint screen_text_uloc = glGetUniformLocation(effect.program, "screen_texture");
	GLuint dead_timer_uloc = glGetUniformLocation(effect.program, "new_level_timer");
	GLuint should_darken_uloc = glGetUniformLocation(effect.program, "should_darken");
	bool should_darken = m_new_level_fade > MAX_FADE_TIME / 2;
	glUniform1i(should_darken_uloc, should_darken);
	glUniform1f(dead_timer_uloc, (m_new_level_fade > 0) ? (float)(m_new_level_fade * 0.021f) : -1);
	glUniform1i(screen_text_uloc, 0);

	// Draw the screen texture on the quad geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glDisableVertexAttribArray(0);
}
