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
#include "visualize_cubemap.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"
#include <stdexcept>

namespace lt::method
{

visualize_cubemap::node::node(texture* cubemap, vec3 pos, float radius)
: cubemap(cubemap), pos({pos}), radius(radius), first_index(0)
{
    if(!cubemap || cubemap->get_target() != GL_TEXTURE_CUBE_MAP)
        throw std::runtime_error(
            "Given cubemap texture's target isn't GL_TEXTURE_CUBE_MAP"
        );
}

visualize_cubemap::node::node(
    texture* cubemap_array,
    const std::vector<vec3>& pos,
    float radius,
    unsigned first_index
): cubemap(cubemap_array), pos(pos), radius(radius), first_index(first_index)
{
    if(!cubemap || cubemap->get_target() != GL_TEXTURE_CUBE_MAP_ARRAY)
        throw std::runtime_error(
            "Given cubemap array texture's target isn't "
            "GL_TEXTURE_CUBE_MAP_ARRAY"
        );
}

visualize_cubemap::visualize_cubemap(
    render_target& target,
    resource_pool& pool,
    render_scene* scene,
    const std::vector<node>& cubemaps
):  target_method(target),
    visualize_shader(pool.get_shader(
        shader::path{"generic.vert", "visualize_cubemap.frag"}
    )),
    scene(scene),
    sphere(common::ensure_patched_sphere_primitive(pool, 10)),
    linear_sampler(common::ensure_linear_sampler(pool)),
    cubemaps(cubemaps)
{}

void visualize_cubemap::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* visualize_cubemap::get_scene() const
{
    return this->scene;
}

void visualize_cubemap::set_cubemaps(const std::vector<node>& cubemaps)
{
    this->cubemaps = cubemaps;
}

std::vector<visualize_cubemap::node>& visualize_cubemap::get_cubemaps()
{
    return cubemaps;
}

const std::vector<visualize_cubemap::node>&
visualize_cubemap::get_cubemaps() const
{
    return cubemaps;
}

void visualize_cubemap::execute()
{
    target_method::execute();

    if(!visualize_shader || cubemaps.size() == 0 || !scene) return;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glFrontFace(GL_CW);

    glm::mat4 vp =
        cam->get_projection() * glm::inverse(cam->get_global_transform());

    for(node n: cubemaps)
    {
        bool is_array = n.cubemap->get_target() == GL_TEXTURE_CUBE_MAP_ARRAY;

        shader::definition_map def;
        if(is_array) def["CUBEMAP_ARRAY"];

        shader* s = visualize_shader->get(def);
        s->set("cubemap", linear_sampler.bind(*n.cubemap, 0));
        s->set("m", mat4(1.0f));

        int max_index = min(
            (int)n.pos.size(),
            max((int)n.cubemap->get_dimensions().z, 1) - (int)n.first_index
        );

        for(int i = 0; i < max_index; ++i)
        {
            mat4 m = glm::translate(n.pos[i]) * glm::scale(vec3(n.radius));
            mat4 mvp = vp * m;
            s->set<int>("array_index", i + n.first_index);
            s->set("mvp", mvp);
            sphere.draw();
        }
    }

    glFrontFace(GL_CCW);
}

std::string visualize_cubemap::get_name() const
{
    return "visualize_cubemap";
}

} // namespace lt::method
