#include "LightBeamParticle.hpp"

void LightBeamParticle::update(float ms) {
	const float OPACITY_STEP = 0.14f;

	m_opacity -= OPACITY_STEP * (ms / 100);
	if (m_opacity < 0) {
		destroyed = true;
		destroy();
	}
}

bool LightBeamParticle::init(float x_pos, float y_pos) {
	
	m_scale.x = 0.4f;
	m_scale.y = 0.4f;

	m_position.x = (float)x_pos;
	m_position.y = (float)y_pos;

	darkness_modifier = 1.0f;
	return true;
}

void LightBeamParticle::set_texture(Texture* tex) {
	texture = tex;

	// The position corresponds to the center of the texture
	float wr = texture->width * 0.5f;
	float hr = texture->height * 0.5f;



	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.02f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.02f };
	vertices[1].texcoord = { 1.f, 1.f };
	vertices[2].position = { +wr, -hr, -0.02f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.02f };
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
		std::cout << "glsl errors occurred when setting light particle texture" << std::endl;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		std::cout << "couldn't load texture shader for light particles" << std::endl;
}

void LightBeamParticle::draw(const mat3& projection) {
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform_begin();
	transform_translate(m_position);
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
	glBindTexture(GL_TEXTURE_2D, texture->id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
	EntityColor color = get_color();
	float fvColor[4] = { color.r*darkness_modifier, color.g*darkness_modifier, color.b*darkness_modifier, color.a*m_opacity};

	glUniform4fv(color_uloc, 1, fvColor);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	
}

bool LightBeamParticle::is_destroyed() {
	return destroyed;
}