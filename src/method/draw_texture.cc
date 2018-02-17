#include "draw_texture.hh"

method::draw_texture::draw_texture(
    render_target& target,
    shader_store& store,
    texture* tex
): target_method(target),
   quad(vertex_buffer::create_square(target.get_context())),
   draw_shader(store.get(shader::path{"generic.vert", "draw_texture.frag"},
               quad.get_definitions())),
   transform(1.0f),
   tex(tex)
{
}

method::draw_texture::~draw_texture()
{
}

void method::draw_texture::set_transform(glm::mat4 transform)
{
    this->transform = transform;
}

void method::draw_texture::set_texture(texture* tex)
{
    this->tex = tex;
}

void method::draw_texture::execute()
{
    target_method::execute();

    if(!draw_shader || !tex) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    draw_shader->bind();

    draw_shader->set("mvp", transform);
    draw_shader->set("tex", tex->bind(0));

    quad.draw();
}
