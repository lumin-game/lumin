//
// Created by Sherry Yuan on 2019-03-08.
//

#include <random>
#include "lantern.hpp"
#include "CollisionManager.hpp"

bool Lantern::init(float x_pos, float y_pos) {
    Entity::init(x_pos, y_pos);
    std::random_device rand;
    std::mt19937 gen(rand());
    std::uniform_real_distribution<> dis(-FIREFLY_DISTRIBUTION, FIREFLY_DISTRIBUTION);
    for (int i = 0; i < FIREFLY_COUNT; ++i) {
        fireflies.emplace_back(SingleFirefly((float) dis(gen), (float) dis(gen)));
    }

    lightMesh.init();
    return true;
}

void Lantern::update(float ms) {

    if (ms_since_activation < MAX_MS_SINCE_ACTIVATION && get_lit()) {
        ms_since_activation += ms;
        num_fireflies_drawn = std::min(FIREFLY_COUNT - 1, (int) (ms_since_activation / MS_BETWEEN_SPAWN));
    }
    for (SingleFirefly &firefly : fireflies) {
        firefly.position += firefly.velocity * ms;
        firefly.position = {std::clamp(firefly.position.x, -firefly.FIREFLY_MAX_RANGE, firefly.FIREFLY_MAX_RANGE),
                            std::clamp(firefly.position.y, -firefly.FIREFLY_MAX_RANGE, firefly.FIREFLY_MAX_RANGE)};
        firefly.velocity += firefly.CalculateForce(fireflies) * ms;
    }
}

void Lantern::draw(const mat3 &projection) {
    Entity::draw(projection);
    if (get_lit()) {
        SingleFirefly::ParentData fireflyData;
        fireflyData.m_position = m_position;
        fireflyData.m_screen_pos = vec2{m_screen_pos.x, m_screen_pos.y + 15};

        int i = 0;
        for (SingleFirefly& firefly : fireflies)
        {
            if (i > num_fireflies_drawn) {
                break;
            }

            firefly.parent = fireflyData;
            firefly.draw(projection);
            i++;
        }

        LightMesh::ParentData lightData;
        lightData.m_position = m_position;
        lightData.m_screen_pos = m_screen_pos;
        lightMesh.SetParentData(lightData);
        lightMesh.draw(projection);
    }
}

void Lantern::destroy() {
    Entity::destroy();
    Firefly::destroy();
}

void Lantern::activate() {
    set_lit(true);
}
