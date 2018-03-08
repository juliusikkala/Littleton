#include "draw_texture.hh"
#include "render_target.hh"
#include "texture.hh"
#include "shader_pool.hh"

method::draw_texture::draw_texture(
    render_target& target,
    shader_pool& pool,
    texture* tex
):  target_method(target),
    quad(vertex_buffer::create_square(target.get_context())),
    color_sampler(
        target.get_context(),
        GL_NEAREST,
        GL_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    transform(1.0f),
    tex(tex)
{
    shader::definition_map def;
    quad.update_definitions(def);

    draw_shader = pool.get(
        shader::path{"generic.vert", "draw_texture.frag"},
        def
    );
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
    draw_shader->set("tex", color_sampler.bind(tex->bind(0)));

    quad.draw();
}

std::string method::draw_texture::get_name() const
{
    return "draw_texture";
}
