#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <vector>
#include <cmath>
#include <cstdlib>

// glfw
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// Simple utility macros to avoid mistyping directory name, name has to be a string literal
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "project_path.hpp"

#define shader_path(name) PROJECT_SOURCE_DIR "./shaders/" name

#define data_path PROJECT_SOURCE_DIR "./data"
#define textures_path(name)  data_path "/textures/" name
#define spritesheet_path(name)  data_path "/spritesheets/" name
#define audio_path(name) data_path  "/audio/" name
#define levels_path(name) data_path "/levels/" name
#define fonts_path(name) data_path "/fonts/" name

// Not much math is needed and there are already way too many libraries linked (:
// If you want to do some overloads..
struct vec2
{ 
	float x, y; 

	float Magnitude() const
	{
		float sqr = x * x + y * y;
		return std::sqrt(sqr);
	}

	vec2 Direction() const
	{
		float distance = Magnitude();
		if (std::abs(distance) < 0.000001)
		{
			return { 0.f, 0.f };
		}
		return *this * (1 / distance);
	}

	vec2 operator+ (const vec2& other) const
	{
		return { x + other.x, y + other.y };
	}

	vec2 operator- (const vec2& other) const
	{
		return { x - other.x, y - other.y };
	}

	void operator+= (const vec2& other)
	{
		x += other.x;
		y += other.y;
	}

	vec2 operator*(float scalar) const
	{
		return { x * scalar,y * scalar };
	}
};
struct vec3 { float x, y, z; };
struct mat3 { vec3 c0, c1, c2; };

// Utility functions
float dot(vec2 l, vec2 r);
float dot(vec3 l, vec3 r);
mat3  mul(const mat3& l, const mat3& r);
vec2  normalize(vec2 v);

// OpenGL utilities
// cleans error buffer
void gl_flush_errors();
bool gl_has_errors();

// Single Vertex Buffer element for non-textured meshes (colored.vs.glsl)
struct Vertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Texture wrapper
struct Texture
{
	Texture();
	~Texture();

	GLuint id;
	GLuint depth_render_buffer_id;
	int width;
	int height;
	
	// Loads texture from file specified by path
	bool load_from_file(const char* path);
	// Screen texture
	bool create_from_screen(GLFWwindow const * const window);
	bool is_valid()const; // True if texture is valid
};

// A Mesh is a collection of a VertexBuffer and an IndexBuffer. A VAO
// represents a Vertex Array Object and is the container for 1 or more Vertex Buffers and 
// an Index Buffer
struct Mesh
{
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
};

// Container for Vertex and Fragment shader, which are then put(linked) together in a
// single program that is then bound to the pipeline.
struct Effect
{
	bool load_from_file(const char* vs_path, const char* fs_path);
	void release();

	GLuint vertex;
	GLuint fragment;
	GLuint program;
};

// Helper container for all the information we need when rendering an object together
// with its transform.
struct Renderable
{
	Mesh mesh;
	Effect effect;
	mat3 transform;

	// projection contains the orthographic projection matrix. As every Renderable::draw()
	// renders itself it needs it to correctly bind it to its shader.
	virtual void draw(const mat3& projection) = 0;

	// gl Immediate mode equivalent, see the Rendering and Transformations section in the
	// specification pdf
	void transform_begin();
	void transform_scale(vec2 scale);
	void transform_rotate(float radians);
	void transform_translate(vec2 pos);
	void transform_end();
};

struct ParametricLine
{
	// A line is:
	// x = x_0 + x_t * t
	// y = y_0 + y_t * t
	// where 0 <= t <= 1
	float x_0;
	float x_t;
	float y_0;
	float y_t;
};

typedef std::vector<ParametricLine> ParametricLines;

struct EntityLines
{
	class Entity* entity;
	ParametricLines boundaryLines;
	ParametricLines lightCollisionLines;
};

enum StaticTile
{
	WALL,
	GLASS,
	LIGHTWALL,
	DARKWALL,
	FOG,
	FIREFLY,
	PLAYER
};