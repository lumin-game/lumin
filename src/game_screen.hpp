#pragma once

#include "common.hpp"

class GameScreen : public Renderable
{
public:
	virtual ~GameScreen() { GameScreen::destroy(); };

	virtual const char* get_texture_path() const = 0;
	// Creates all the associated render resources and default transform
	virtual bool init(vec2 screen);

	// Releases all the associated resources
	virtual void destroy();

	void draw(const mat3& projection)override;

	virtual void set_position(vec2 position, vec2 offset = { 0, 0 });

	virtual void set_translation_scale();

	virtual void set_translation_position(vec2 screen, bool is_left);

	vec2 get_bounding_box()const;

	vec2 calculate_position(vec2 screen, bool is_left);

protected:
	Texture screen_texture;
	vec2 m_scale;
	vec2 m_position;
};
