#include "screen.hpp"

#include <iostream>

Texture Screen::menu_texture;

bool Screen::init() {

	// Since we are not going to apply transformation to this screen geometry
	// The coordinates are set to fill the standard openGL window [-1, -1 .. 1, 1]
	// Make the size slightly larger then the screen to crop the boundary

	m_render_menu = get_render_menu();

	if (!m_render_menu) {
		return render_screen();
	} else {
		return render_menu();
	}
}

void Screen::destroy() {
	glDeleteBuffers(1, &mesh.vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Screen::draw(const mat3& projection) {

	if (!m_render_menu) {
		draw_screen();
	} else {
		draw_menu(projection);
	}
}

bool Screen::render_menu(){
	if (!menu_texture.is_valid())
		{
			if (!menu_texture.load_from_file(textures_path("level_screen.png")))
			{
				fprintf(stderr, "Failed to load level screen texture!");
				return false;
			}
		}
		// The position corresponds to the center of the texture
		float wr = menu_texture.width * 0.5f;
		float hr = menu_texture.height * 0.5f;

		TexturedVertex vertices[4];
		vertices[0].position = { -wr, +hr, 0.1f };
		vertices[0].texcoord = { 0.f, 1.f };
		vertices[1].position = { +wr, +hr, 0.1f };
		vertices[1].texcoord = { 1.f, 1.f };
		vertices[2].position = { +wr, -hr, 0.1f };
		vertices[2].texcoord = { 1.f, 0.f };
		vertices[3].position = { -wr, -hr, 0.1f };
		vertices[3].texcoord = { 0.f, 0.f };

		// counterclockwise as it's the default opengl front winding direction
		uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

		// Clearing errors
		gl_flush_errors();

		// Vertex Buffer creation
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

		// Index Buffer creation
		glGenBuffers(1, &mesh.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

		// Vertex Array (Container for Vertex + Index buffer)
		glGenVertexArrays(1, &mesh.vao);
		if (gl_has_errors())
			return false;

		// Loading shaders
		if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
			return false;

		// Setting initial values, scale is negative to make it face the opposite way
		// 1.0 would be as big as the original texture
		// no scaling for now
		// m_scale.x = 1.f;
		// m_scale.y = 1.f;

		return true;
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

void Screen::draw_menu(const mat3& projection){
	transform_begin();
	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()
	transform_translate(m_parent.m_screen_pos);
	transform_scale(m_scale);

	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint light_radius = glGetUniformLocation(effect.program, "lightRadius");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, menu_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	float color[] = { 1.f, 0.f, 0.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	glUniform1f(light_radius, 200.f);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
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

bool Screen::get_render_menu(){
	return m_render_menu;
}

void Screen::set_render_menu(bool m_is_m_pressed){
	m_render_menu = m_is_m_pressed;
}

void Screen::reset() {
	m_render_menu = false;
}