#include "firefly.hpp"
#include "CollisionManager.hpp"
#include <random>

vec2 Firefly::SingleFirefly::CalculateForce(std::vector<SingleFirefly>& fireflies) const
{
	vec2 force = { 0.f, 0.f };
	const float constant = 0.000005f;
	const float radialDistance = 3.f;
	const float rotationNoiseMod = 30.f;
	const int rotationNoiseMin = 10;
	const float maxNoise = 5.0f;

	vec2 toCenter = position.Direction() * -1;
	force += toCenter * (constant * position.Magnitude());

	if (position.Magnitude() < radialDistance)
	{
		vec2 noise = { 0.f, 0.f };
		for (const SingleFirefly& firefly : fireflies)
		{
			vec2 difference = position - firefly.position;
			float distance = difference.Magnitude();
			if (std::abs(distance) < 0.000001)
			{
				continue;
			}
			noise += difference.Direction() * (constant * distance);
		}

		int randClockwise = std::rand() % 2 > 0 ? 1 : -1;
		int randMagnitude = std::rand() % rotationNoiseMin;
		vec2 perpendicular = { -position.y, position.x };
		noise += perpendicular * (float) randClockwise * (constant * (float) randMagnitude * rotationNoiseMod);
		noise = noise * (1 / maxNoise);
		force += noise;
	}

	return force;
}

bool Firefly::SingleFirefly::init()
{
	std::vector<Vertex> vertices;

	Vertex vertex;
	vertex.color = { 1.f, 1.f, 1.f };

	vertex.position = { FIREFLY_RADIUS, FIREFLY_RADIUS, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { FIREFLY_RADIUS, -FIREFLY_RADIUS, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { -FIREFLY_RADIUS, -FIREFLY_RADIUS, -0.02f };
	vertices.push_back(vertex);
	vertex.position = { -FIREFLY_RADIUS, FIREFLY_RADIUS, -0.02f };
	vertices.push_back(vertex);

	uint16_t indices[] = { 1, 3, 2, 0, 3, 1 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("firefly.vs.glsl"), shader_path("firefly.fs.glsl")))
		return false;

	return true;
}

void Firefly::SingleFirefly::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	effect.release();
}

void Firefly::SingleFirefly::update(float ms, std::vector<SingleFirefly>& fireflies)
{
	position += velocity * ms;
	position = { std::clamp(position.x, -FIREFLY_MAX_RANGE, FIREFLY_MAX_RANGE), std::clamp(position.y, -FIREFLY_MAX_RANGE, FIREFLY_MAX_RANGE) };
	velocity += CalculateForce(fireflies) * ms;
}

void Firefly::SingleFirefly::draw(const mat3& projection)
{
	transform_begin();

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// transform_translate()
	// transform_rotate()
	// transform_scale()

	transform_translate(parent.m_screen_pos + position);
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
	GLint fireflyRadius = glGetUniformLocation(effect.program, "fireflyRadius");


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
	glUniform1fv(fireflyRadius, 1, &FIREFLY_RADIUS);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

bool Firefly::init()
{
	m_scale.x = 1.f;
	m_scale.y = 1.f;

	m_position = { 0.f, 0.f };
	m_screen_pos = m_position;

	std::random_device rand;
	std::mt19937 gen(rand());
	std::uniform_real_distribution<> dis(-FIREFLY_DISTRIBUTION, FIREFLY_DISTRIBUTION);
	for (int i = 0; i < FIREFLY_COUNT; ++i)
	{
		fireflies.push_back(SingleFirefly((float) dis(gen), (float) dis(gen)));
	}

	if (!lightMesh.init())
	{
		return false;
	}

	return true;
}

void Firefly::destroy()
{
	for (SingleFirefly& firefly : fireflies)
	{
		firefly.destroy();
	}

	lightMesh.destroy();
}

void Firefly::update(float ms)
{
	const float VELOCITY_STEP = 0.025f;
	const float VELOCITY_SLOWING_STEP = 0.04f;

	if (CollisionManager::GetInstance().IsHitByLight(m_position)) { // Follow closest light source if one is in sight
		vec2 destLight = CollisionManager::GetInstance().getClosestVisibleLightSource(m_position);

		if (destLight.x > m_position.x) {
			m_velocity.x += VELOCITY_STEP * (ms/100);
		}
		else if (destLight.x < m_position.x) {
			m_velocity.x -= VELOCITY_STEP * (ms / 100);
		}

		if (destLight.y > m_position.y) {
			m_velocity.y += VELOCITY_STEP * (ms / 100);
		}
		else if (destLight.y < m_position.y) {
			m_velocity.y -= VELOCITY_STEP * (ms / 100);
		}
	}
	else { // Slow fireflies down if a light is not in sight
		if (m_velocity.y > 0) {
			m_velocity.y -= fmin(m_velocity.y, VELOCITY_SLOWING_STEP * (ms / 100));
		}
		else if (m_velocity.y < 0) {
			m_velocity.y -= fmax(m_velocity.y, -VELOCITY_SLOWING_STEP * (ms / 100));
		}

		if (m_velocity.x > 0) {
			m_velocity.x -= fmin(m_velocity.x, VELOCITY_SLOWING_STEP * (ms / 100));
		}
		else if (m_velocity.x < 0) {
			m_velocity.x -= fmax(m_velocity.x, -VELOCITY_SLOWING_STEP * (ms / 100));
		}
	}
	float xDist = m_velocity.x * ms;
	float yDist = m_velocity.y * ms;

	CollisionManager::CollisionResult collisionResult = CollisionManager::GetInstance().BoxTrace(10, 10, m_position.x, m_position.y, xDist, yDist);

	m_position.x = collisionResult.resultXPos;
	m_position.y = collisionResult.resultYPos;

	if (collisionResult.hitGround || collisionResult.hitCeiling)
	{
		m_velocity.y = 0.f;
	}
	//TODO eventually also check for hits on X direction and set m_vel.x to 0 if they happen

	for (SingleFirefly& firefly : fireflies)
	{
		firefly.update(ms, fireflies);
	}
}

void Firefly::draw(const mat3& projection)
{
	SingleFirefly::ParentData fireflyData;
	fireflyData.m_position = m_position;
	fireflyData.m_screen_pos = m_screen_pos;

	for (SingleFirefly& firefly : fireflies)
	{
		firefly.parent = fireflyData;
		firefly.draw(projection);
	}

	LightMesh::ParentData lightData;
	lightData.m_position = m_position;
	lightData.m_screen_pos = m_screen_pos;
	lightMesh.SetParentData(lightData);
	lightMesh.draw(projection);
}

vec2 Firefly::get_position() const {
	return m_position;
}

void Firefly::set_position(vec2 position)
{
	m_position = position;
}

void Firefly::set_screen_pos(vec2 position) {
	m_screen_pos = position;
}