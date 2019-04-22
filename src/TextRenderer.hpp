#include <common.hpp>

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
	void drawText(std::string text, GLfloat x, GLfloat y, GLfloat scale, float color[]);

private:
	struct Character {
		GLuint textureID;
		glm::ivec2 size;
		glm::ivec2 Bearing;
		GLuint Advance;
	};

	std::map<GLchar, Character> Characters;
	vec2 m_position;
	vec2 m_scale = { 1.f, 1.f };
};