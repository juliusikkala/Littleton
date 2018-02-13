#include "sky.hh"

method::sky::sky(
    render_target& target,
    shader_store& store,
    render_scene* scene
): target_method(target),
   sky_shader(store.get(shader::path{"fullscreen.vert", "sky.frag"}, {})),
   scene(scene),
   fullscreen_quad(vertex_buffer::create_square(target.get_context()))
{
}

void method::sky::set_scene(render_scene* s)
{
    scene = s;
}

render_scene* method::sky::get_scene() const
{
    return scene;
}

void method::sky::execute()
{
    target_method::execute();

    if(!sky_shader || !scene) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);

    sky_shader->bind();
    fullscreen_quad.draw();
}
