/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "visualize_gbuffer.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"

namespace
{
using namespace lt;
using namespace lt::method;

void render_visualizer(
    const gbuffer* buf,
    visualize_gbuffer::options::visualizer v,
    multishader* ms,
    const primitive& quad,
    camera* cam
){
    shader* s;
    switch(v)
    {
    case visualize_gbuffer::options::DEPTH:
        s = ms->get({{"SHOW_DEPTH", ""}});
        break;
    case visualize_gbuffer::options::POSITION:
        s = ms->get({{"SHOW_POSITION", ""}});
        break;
    case visualize_gbuffer::options::NORMAL:
        s = ms->get({{"SHOW_NORMAL", ""}});
        break;
    case visualize_gbuffer::options::COLOR:
        s = ms->get({{"SHOW_COLOR", ""}});
        break;
    case visualize_gbuffer::options::ROUGHNESS:
        s = ms->get({{"SHOW_ROUGHNESS", ""}});
        break;
    case visualize_gbuffer::options::METALLIC:
        s = ms->get({{"SHOW_METALLIC", ""}});
        break;
    case visualize_gbuffer::options::IOR:
        s = ms->get({{"SHOW_IOR", ""}});
        break;
    case visualize_gbuffer::options::MATERIAL:
        s = ms->get({{"SHOW_MATERIAL", ""}});
        break;
    case visualize_gbuffer::options::LIGHTING:
        s = ms->get({{"SHOW_LIGHTING", ""}});
        break;
    case visualize_gbuffer::options::INDIRECT_LIGHTING:
        s = ms->get({{"SHOW_INDIRECT_LIGHTING", ""}});
        break;
    default:
        throw std::runtime_error("Unknown visualizer type");
    }
    s->bind();
    unsigned texture_index = 0;
    buf->set_uniforms(s, texture_index);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    quad.draw();
}

}

namespace lt::method
{

method_options<visualize_gbuffer>::method_options()
: visualizers({POSITION, NORMAL, COLOR, MATERIAL})
{}

method_options<visualize_gbuffer>::method_options(visualizer full)
: visualizers({full})
{}

method_options<visualize_gbuffer>::method_options(
    visualizer topleft,
    visualizer topright,
    visualizer bottomleft,
    visualizer bottomright
): visualizers({topleft, topright, bottomleft, bottomright})
{}

visualize_gbuffer::visualize_gbuffer(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target), scene_method(scene), options_method(opt), buf(&buf),
    visualize_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "visualize.frag"}
    )),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void visualize_gbuffer::execute()
{
    target_method::execute();

    const auto [visualizers] = opt;

    if(!visualize_shader || visualizers.size() == 0 || !has_all_scenes()) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    unsigned texture_index = 0;
    buf->bind_textures(fb_sampler, texture_index);
    buf->set_draw(gbuffer::DRAW_LIGHTING);

    if(visualizers.size() == 1)
    {
        render_visualizer(buf, visualizers[0], visualize_shader, quad, cam);
    }
    else if(visualizers.size() == 4)
    {
        glm::uvec2 size = get_target().get_size();
        glm::uvec2 half_size = size/2u;

        glViewport(0, half_size.y, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[0], visualize_shader, quad, cam);

        glViewport(half_size.x, half_size.y, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[1], visualize_shader, quad, cam);

        glViewport(0, 0, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[2], visualize_shader, quad, cam);

        glViewport(half_size.x, 0, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[3], visualize_shader, quad, cam);

        glViewport(0, 0, size.x, size.y);
    }
}

std::string visualize_gbuffer::get_name() const
{
    return "visualize_gbuffer";
}

} // namespace lt::method
