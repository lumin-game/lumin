#include "switch.hpp"
#include "CollisionManager.hpp"
#include "LightBeam.hpp"

bool Switch::init(float x_pos, float y_pos) {
	if (!particle_texture.load_from_file(textures_path("light_particle.png"))) {
		fprintf(stderr, "Failed to load switch particle texture!");
		return false;
	}

	Entity::init(x_pos, y_pos);
}

void Switch::activate() {
	Mix_PlayChannel(-1, get_sound(), 0);
	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->activate();

			LightBeam* lightBeam = new LightBeam();
			lightBeam->init(m_position.x, m_position.y);
			lightBeam->setParameters(entity->get_position(), this);
			lightBeam->set_lit(true);
			light_beams.insert(lightBeam);
		}
	}
}

void Switch::deactivate() {
	if (!mToggleSwitch) {
		return;
	}

	for (auto* entity : m_entities) {
		if (entity != nullptr) {
			entity->deactivate();
		}
		else {
			m_entities.erase(entity);
		}
	}
}

void Switch::update(float ms) {

	for (std::set<Entity*>::iterator it = light_beams.begin(); it != light_beams.end();) {
		if (LightBeam* beam = dynamic_cast<LightBeam*>(*it)) {
			if (beam == nullptr || beam->is_destroyed()) {
				it = light_beams.erase(it);
				//Do i need to deallocate the memory here by calling delete?
			}
			else {
				(*it)->update(ms);
				++it;
			}
		}
	}

	for (std::set<LightBeamParticle*>::iterator it = light_beam_particles.begin(); it != light_beam_particles.end();) {
		if ((*it) == nullptr || (*it)->is_destroyed()) {
			it = light_beam_particles.erase(it);
			//Do i need to deallocate the memory here by calling delete?
		}
		else {
			(*it)->update(ms);
			++it;
		}
	}
}

void Switch::draw(const mat3& projection) {
	Entity::draw(projection);

	for (std::set<LightBeamParticle*>::iterator it = light_beam_particles.begin(); it != light_beam_particles.end();) {
		if ((*it) == nullptr || (*it)->is_destroyed()) {
			it = light_beam_particles.erase(it);
			//Do i need to deallocate the memory by calling delete?
		}
		else {
			(*it)->draw(projection);
			++it;
		}
	}
}

void Switch::add_beam_particle(LightBeamParticle* particle) {
	// sending all particles the texture from switch lets us only load the texture from file once per switch instead of once per particle
	particle->set_texture(&particle_texture); 

	light_beam_particles.insert(particle);
}


void Switch::set_toggle_switch(bool isToggle) {
	mToggleSwitch = isToggle;

	// Reinitialize the entity so that we get the proper texture (hehe)
	Entity::init(get_position().x, get_position().y);
}
