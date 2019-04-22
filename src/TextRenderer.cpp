#include "TextRenderer.hpp"

bool TextRenderer::init() {
	if (FT_Init_FreeType(&ft)) // All functions return a value different than 0 whenever an error occurred
	{
		fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library");
		return false;
	}

	if (FT_New_Face(ft, fonts_path("arial.ttf"), 0, &face))
	{
		fprintf(stderr, "ERROR::FREETYPE: Failed to load font");
		return false;
	}
	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Loading shaders
	if (!effect.load_from_file(shader_path("text.vs.glsl"), shader_path("text.fs.glsl")))
		return false;

	attribute_coord = glGetAttribLocation(effect.program, "coord");
	uniform_tex = glGetUniformLocation(effect.program, "tex");
	uniform_color = glGetUniformLocation(effect.program, "color");

	glGenBuffers(1, &vbo);

	return true;
}

void TextRenderer::destroy() {
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glDeleteBuffers(1, &vbo);

	effect.release();
}

void TextRenderer::draw(const mat3& projection) {
	FT_GlyphSlot g = face->glyph;

	char* text = "Current level: ";
	// Setting shaders
	glUseProgram(effect.program);

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	GLfloat red[4] = { 1, 1, 1, 1 };
	glUniform4fv(uniform_color, 1, red);

	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uniform_tex, 0);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/* Set up the VBO for our vertex data */
	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	float sx = 2.f / 1200;
	float sy = 2.f / 800;

	//GLfloat x = -1 + 8 * sx;
	//GLfloat y = 1 - 50 * sy;

	GLfloat x = 0.f, y = 0.f;

	const char *p;
	for (p = text; *p; p++) {
		/* Try to load and render the character */
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
			fprintf(stderr, "ERROR::FREETYPE: Failed to load font glyph");
			continue;
		}

		/* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		/* Calculate the vertex and texture coordinates */
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{x2, -y2, 0, 0},
			{x2 + w, -y2, 1, 0},
			{x2, -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		/* Draw the character on the screen */
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisableVertexAttribArray(attribute_coord);
	glDeleteTextures(1, &tex);
	glDisable(GL_TEXTURE_2D);

	/*
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y + (ch.size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 0.0 },
			{ xpos,     ypos,       0.0, 0.0 },

			{ xpos,     ypos + h,   0.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 1.0 },
			{ xpos + w, ypos,       1.0, 0.0 }
		};
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
		// Render quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
	}
	*/
	//drawText("Hello world", 425.f, -364.f, 10.f, color);
}

void TextRenderer::drawText(std::string text, GLfloat x, GLfloat y, GLfloat scale, float color[]) {
	
}
