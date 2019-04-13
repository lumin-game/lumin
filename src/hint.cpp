#include "hint.hpp"


void Hint::set_hint_path(const std::string &hint_path) {
    m_hint_path = PROJECT_SOURCE_DIR "./data/hints/" + hint_path;
    if (get_lit_texture_path() != nullptr && !lit_texture.load_from_file(get_texture_path())) {
        fprintf(stderr, "Failed to load hint texture!");
    }
    texture = &lit_texture;

    // Resize and reposition texture
    float wr = texture->width * 0.5f;
    float hr = texture->height * 0.5f;

    TexturedVertex vertices[4];
    vertices[0].position = { -wr, +hr, -0.02f };
    vertices[0].texcoord = { 0.f, 1.f };
    vertices[1].position = { +wr, +hr, -0.02f };
    vertices[1].texcoord = { 1.f, 1.f };
    vertices[2].position = { +wr, -hr, -0.02f };
    vertices[2].texcoord = { 1.f, 0.f };
    vertices[3].position = { -wr, -hr, -0.02f };
    vertices[3].texcoord = { 0.f, 0.f };

    // counterclockwise as it's the default opengl front winding direction
    uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

    // Clearing errors
    gl_flush_errors();

    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);
}