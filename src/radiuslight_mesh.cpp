  // Header
#include "radiuslight_mesh.hpp"
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

bool RadiusLightMesh::init()
{
	m_lightRadius = 300.f;

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("radiuslight.vs.glsl"), shader_path("radiuslight.fs.glsl")))
		return false;

	CollisionManager::GetInstance().RegisterRadiusLight(this);

	return true;
}

// Releases all graphics resources
void RadiusLightMesh::destroy()
{
	CollisionManager::GetInstance().UnregisterRadiusLight(this);

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void RadiusLightMesh::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()

	transform_translate(m_parent.m_position);

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
	GLint show_polygon = glGetUniformLocation(effect.program, "showPolygon");

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
	glUniform1f(light_radius, m_lightRadius);
	glUniform1i(show_polygon, (int) m_enablePolygon);

	// Drawing!
	glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_SHORT, nullptr);
}

void RadiusLightMesh::predraw()
{
	// Recreate polygonial mesh based on objects that block light around us
	UpdateVertices();
}

void RadiusLightMesh::UpdateVertices()
{
	// Update our collision equations based on where we are in the world
	// CollisionManager is friend
	const CollisionManager& colManager = CollisionManager::GetInstance();

	// Get all relevant entities in radius
	std::vector<Entity*> entities = colManager.GetEntitiesInRange(m_parent.m_position.x, m_parent.m_position.y, m_lightRadius);

	// Ordered points is not ordered yet, but we will sort it at the end, hence they are called orderedPoints
	std::vector<vec2> orderedPoints;

	// Add the four corners of the bounding box of the light, in case we do not have any entities near us, we still render the light
	const vec2 topRight = { m_lightRadius, m_lightRadius };
	const vec2 topLeft = { -m_lightRadius, m_lightRadius };
	const vec2 bottomRight = { m_lightRadius, -m_lightRadius };
	const vec2 bottomLeft = { -m_lightRadius, -m_lightRadius };

	orderedPoints.push_back(topRight);
	orderedPoints.push_back(topLeft);
	orderedPoints.push_back(bottomRight);
	orderedPoints.push_back(bottomLeft);

	// Process each entity's corners to orderedPoints
	for (Entity* entity : entities)
	{
		vec2 posToEntity = { entity->get_position().x - m_parent.m_position.x, entity->get_position().y - m_parent.m_position.y };

		const float xRadius = entity->get_bounding_box().x / 2;
		const float yRadius = entity->get_bounding_box().y / 2;

		vec2 topRight = { posToEntity.x + xRadius, posToEntity.y - yRadius };
		vec2 topLeft = { posToEntity.x - xRadius, posToEntity.y - yRadius };
		vec2 bottomRight = { posToEntity.x + xRadius, posToEntity.y + yRadius };
		vec2 bottomLeft = { posToEntity.x - xRadius, posToEntity.y + yRadius };

		std::vector<vec2> entityPoints = { topRight, topLeft, bottomLeft, bottomRight };

		// For each vertex, add two more lines slightly to the left and right
		// https://ncase.me/sight-and-light/
		for (const vec2& corner : entityPoints)
		{
			const float smallRadian = 0.0001f;
			const float angle = std::atan2(corner.y, corner.x);
			const float clockwise = angle - smallRadian;
			const float antiClockwise = angle + smallRadian;
			const vec2 clockwisePoint = { cos(clockwise), sin(clockwise) };
			const vec2 antiClockwisePoint = { cos(antiClockwise), sin(antiClockwise) };

			orderedPoints.push_back(clockwisePoint * m_lightRadius * 2);
			orderedPoints.push_back(corner);
			orderedPoints.push_back(antiClockwisePoint * m_lightRadius * 2);
		}
	}

	// Sort points by smallest angle to largest angle to the positive x-axis
	std::sort(orderedPoints.begin(), orderedPoints.end(), [](const vec2& point1, const vec2& point2)
	{
		float angle1 = std::atan2(-point1.y, point1.x);
		angle1 = angle1 > 0 ? angle1 : 2 * PI + angle1;
		float angle2 = std::atan2(-point2.y, point2.x);
		angle2 = angle2 > 0 ? angle2 : 2 * PI + angle2;

		return angle1 < angle2;
	});

	// Bound lines is a list of entities and their boundary lines
	std::vector<EntityBoundLine> boundLines;
	for (Entity* entity : entities)
	{
		ParametricLines lines;
		for (ParametricLine line : entity->calculate_boundary_equations())
		{
			line.x_0 = line.x_0 - m_parent.m_position.x;
			line.y_0 = line.y_0 - m_parent.m_position.y;

			lines.push_back(line);
		}

		EntityBoundLine boundLine = { entity, lines };
		boundLines.push_back(boundLine);
	}

	// For each point, rayTrace from origin to it. The result will be one vertex for our polygon
	std::vector<vec2> polyVertices;
	for (const vec2& corner : orderedPoints)
	{
		ParametricLine rayTrace;
		rayTrace.x_0 = 0.f;
		rayTrace.x_t = corner.x;
		rayTrace.y_0 = 0.f;
		rayTrace.y_t = corner.y;

		vec2 hitPos = { rayTrace.x_t, rayTrace.y_t };

		// Keep track of all entities hit
		std::vector<EntityHit> entitiesHit;
		for (EntityBoundLine& entityBounds : boundLines)
		{
			for (ParametricLine lightEq : entityBounds.lines)
			{
				vec2 collisionLocation;
				if (colManager.LinesCollide(rayTrace, lightEq, collisionLocation))
				{
					EntityHit entityHit = { entityBounds.entity, collisionLocation };
					entitiesHit.push_back(entityHit);
				}
			}
		}

		// Sort by increasing distance away from origin
		std::sort(entitiesHit.begin(), entitiesHit.end(), [](const EntityHit& ntHit1, const EntityHit& ntHit2)
		{
			float dist1 = ntHit1.hitLocation.Magnitude();
			float dist2 = ntHit2.hitLocation.Magnitude();
			return dist1 < dist2;
		});

		// Everything before the first light collidable hit has been hit by light, set to lit
		for (EntityHit& entityHit : entitiesHit)
		{
			entityHit.entity->set_lit(true);
			if (entityHit.entity->is_light_collidable())
			{
				hitPos = entityHit.hitLocation;
				break;
			}
		}

		polyVertices.push_back(hitPos);
	}

	// Now create the actual 3d vertex and send to openGL
	const float depth = -0.02f;
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	Vertex vertex;
	vertex.color = { 1.f, 1.f, 1.f };

	vec2 lastPoint = polyVertices.back();

	// Let 0th vertex be origin so we can reuse it
	vertex.position = { 0.f, 0.f, depth };
	vertices.push_back(vertex);

	// Let last vertex inserted to always be our previous vertex
	vertex.position = { lastPoint.x, lastPoint.y, depth };
	vertices.push_back(vertex);

	// Loop
	for (const vec2& hitPoint : polyVertices)
	{
		// We only have to add one vertex per point. However we need to add one triangle per vertex added.
		const int count = vertices.size();
		vertex.position = { hitPoint.x, hitPoint.y, depth };
		vertices.push_back(vertex);

		// Add a triangle, counter-clockwise.
		// As polyVertices are ordered in terms of angle, we know this is always counter-clockwise
		indices.push_back(0); // origin
		indices.push_back(count - 1); // previously processed vertex
		indices.push_back(count); // currently added vertex
	}

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

	indicesToDraw = indices.size();
}

vec2 RadiusLightMesh::get_position() const
{
	return m_parent.m_position;
}

float RadiusLightMesh::getLightRadius() const
{
	return m_lightRadius;
}