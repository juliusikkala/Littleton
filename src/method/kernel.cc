#include "kernel.hh"
#include "render_target.hh"
#include "texture.hh"
#include "shader_pool.hh"

const glm::mat3 method::kernel::SHARPEN = glm::mat3(
     0, -1, 0,
    -1, 5, -1,
     0, -1, 0
);

const glm::mat3 method::kernel::EDGE_DETECT = glm::mat3(
    -1, -1, -1,
    -1, 8, -1,
    -1, -1, -1
)/16.0f;

const glm::mat3 method::kernel::GAUSSIAN_BLUR = glm::mat3(
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
)/16.0f;

const glm::mat3 method::kernel::BOX_BLUR = glm::mat3(
    1, 1, 1,
    1, 1, 1,
    1, 1, 1
)/9.0f;

method::kernel::kernel(
    render_target& target,
    texture& src,
    shader_pool& pool,
    const glm::mat3& k
):  target_method(target), src(&src),
    kernel_shader(
        pool.get_shader(shader::path{"fullscreen.vert", "kernel.frag"}, {})
    ),
    fullscreen_quad(vertex_buffer::create_square(target.get_context())),
    k(k) 
{
}

void method::kernel::set_kernel(const glm::mat3& kernel)
{
    k = kernel;
}

glm::mat3 method::kernel::get_kernel() const
{
    return k;
}

void method::kernel::execute()
{
    target_method::execute();

    if(!kernel_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    kernel_shader->bind();
    kernel_shader->set("kernel", k);
    kernel_shader->set("in_color", src->bind());

    fullscreen_quad.draw();
}

std::string method::kernel::get_name() const
{
    return "kernel";
}
