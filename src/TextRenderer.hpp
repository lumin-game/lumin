#include <common.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <map>
#include <iostream>
#include <glm.hpp>

class TextRenderer : public Renderable{
public:
	TextRenderer() = default;
	bool init();
	void destroy();
	void draw(const mat3& projection)override;
	void drawText(int level, GLfloat x, GLfloat y, GLfloat sx, GLfloat sy);

private:
	FT_Library ft;
	// Load font as face
	FT_Face face;

	GLuint vbo;
	GLuint attribute_coord;
	GLuint uniform_tex;
	GLuint uniform_color;
};