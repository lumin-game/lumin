#include "switchable_entity.hpp"

void SwitchableEntity::on_switch(bool state) {
    m_is_on = state;
    if (state) {
        get_lit_texture_path();
    }

}

bool SwitchableEntity::get_is_on() {
    return m_is_on;
}