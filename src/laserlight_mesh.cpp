  // Header
#include "laserlight_mesh.hpp"
#include "CollisionManager.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <set>

// external
#include "world.hpp"

#define PI 3.14159265

bool LaserLightMesh::init()
{
	m_laserLength = 1000.f;
	m_laserWidth = 15.f;

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("laserlight.vs.glsl"), shader_path("laserlight.fs.glsl")))
		return false;

	CollisionManager::GetInstance().RegisterLaserLight(this);

	return true;
}

// Releases all graphics resources
void LaserLightMesh::destroy()
{
	CollisionManager::GetInstance().UnregisterLaserLight(this);

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void LaserLightMesh::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()

	transform_translate(m_parent.m_position);
	float lightAngle = std::atan2(m_parent.m_mousePosition.y, m_parent.m_mousePosition.x) - PI / 2;
	transform_rotate(lightAngle);
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
	GLint light_width = glGetUniformLocation(effect.program, "lightWidth");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_color_loc = glGetAttribLocation(effect.program, "in_color");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
	
	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	glUniform1f(light_width, m_laserWidth);

	// Recreate polygonial mesh based on objects that block light around us
	int toRender = UpdateVertices();

	// Drawing!
	glDrawElements(GL_TRIANGLES, toRender, GL_UNSIGNED_SHORT, nullptr);
}

int LaserLightMesh::UpdateVertices()
{
	// Find angle where we're going to face
	float lightAngle = std::atan2(m_parent.m_mousePosition.y, m_parent.m_mousePosition.x) - PI/2;
	float cosA = std::cos(lightAngle);
	float sinA = std::sin(lightAngle);

	// Update our collision equations based on where we are in the world
	const CollisionManager& colManager = CollisionManager::GetInstance();

	// Corner points are the vertices of all relevant light objects
	std::vector<vec2> cornerPoints = colManager.CalculateVertices(m_parent.m_position.x, m_parent.m_position.y, m_laserLength);
	
	std::vector<vec2> relevantPoints;
	relevantPoints.push_back({ -m_laserWidth, 0.f });
	relevantPoints.push_back({ m_laserWidth, 0.f });

	// Transform cornerPoints into our rotation in which lightAngle is 'up' (90 deg)
	// We work in this rotation, in which the light starts at (0,0) and goes up to (0,lightLength)
	// The bottom two coordinates of the light is (-lightWidth, 0), (lightWidth, 0)
	for (vec2& point : cornerPoints)
	{
		float newX = point.x * cosA + point.y * sinA;
		float newY = point.x * -sinA + point.y * cosA;
		point.x = newX;
		point.y = newY;

		// Only add the points if they are within the light's area
		if (std::fabs(point.x) <= m_laserWidth && point.y >= 0)
		{
			// Add two extra points slightly to the right and left
			relevantPoints.push_back({ point.x - 0.001f , point.y });
			relevantPoints.push_back(point);
			relevantPoints.push_back({ point.x + 0.001f , point.y });
		}
	}

	// Sort by increasing x
	std::sort(relevantPoints.begin(), relevantPoints.end(), [](const vec2& point1, const vec2& point2)
	{
		return point1.x < point2.x;
	});

	// Grab light blocking equations, we'll have to transform their rotation too
	ParametricLines lightEquations = colManager.CalculateLightEquations(m_parent.m_position.x, m_parent.m_position.y, m_laserLength);
	for (ParametricLine& line : lightEquations)
	{
		vec2 startingPoint = { line.x_0, line.y_0 };
		float newStartX = startingPoint.x * cosA + startingPoint.y * sinA;
		float newStartY = startingPoint.x * -sinA + startingPoint.y * cosA;

		vec2 endPoint = { line.x_t, line.y_t };
		float newEndX = endPoint.x * cosA + endPoint.y * sinA;
		float newEndY = endPoint.x * -sinA + endPoint.y * cosA;

		line.x_0 = newStartX;
		line.y_0 = newStartY;
		line.x_t = newEndX;
		line.y_t = newEndY;
	}

	// Check collisions, these will be the vertices of our polygon
	std::vector<vec2> polyVertices;
	for (const vec2& corner : relevantPoints)
	{
		ParametricLine rayTrace;
		rayTrace.x_0 = corner.x;
		rayTrace.x_t = 0.f;
		rayTrace.y_0 = 0.f;
		rayTrace.y_t = m_laserLength;

		vec2 hitPos = { corner.x, m_laserLength };
		for (const ParametricLine& lightEq : lightEquations)
		{
			// Make sure we use the collision that is the closest to the light origin.
			vec2 collisionLocation;
			if (colManager.LinesCollide(rayTrace, lightEq, collisionLocation))
			{
				if (collisionLocation.Magnitude() < hitPos.Magnitude())
				{
					hitPos = collisionLocation;
				}
			}
		}

		polyVertices.push_back(hitPos);
	}

	// Create vertices, we are still working in our lightAngle rotation coord system
	std::vector<uint16_t> indices;
	std::vector<Vertex> vertices;
	Vertex vertex;
	vertex.color = { 1.f, 1.f, 1.f };
	
	vertex.position = { polyVertices[0].x, 0.f, 0.f };
	vertices.push_back(vertex);
	vertex.position = { polyVertices[0].x, polyVertices[0].y, 0.f };
	vertices.push_back(vertex);

	for (auto it = polyVertices.begin() + 1; it != polyVertices.end(); ++it)
	{
		const int count = vertices.size();

		vertex.position = { it->x, 0.f, 0.f };
		vertices.push_back(vertex);
		vertex.position = { it->x, it->y, 0.f };
		vertices.push_back(vertex);

		// Make a rectangle with these 2 new points and the previous 2 points
		// One triangle
		indices.push_back(count);
		indices.push_back(count - 2);
		indices.push_back(count - 1);

		// Two triangles
		indices.push_back(count);
		indices.push_back(count - 1);
		indices.push_back(count + 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

	return indices.size();
}

vec2 LaserLightMesh::get_position() const
{
	return m_parent.m_position;
}