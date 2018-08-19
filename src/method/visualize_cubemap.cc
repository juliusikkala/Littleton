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
#include "helpers.hh"
#include "scene.hh"
#include <stdexcept>

namespace lt
{
cubemap_visualizer_node::cubemap_visualizer_node(
    texture* cubemap, vec3 pos, float radius
): radius(radius), first_index(0)
{
    set_cubemap(cubemap, pos);
}

cubemap_visualizer_node::cubemap_visualizer_node(
    texture* cubemap_array,
    const std::vector<vec3>& pos,
    float radius,
    unsigned first_index
): radius(radius), first_index(first_index)
{
    set_cubemap(cubemap, pos);
}

void cubemap_visualizer_node::set_cubemap(texture* cubemap, vec3 pos)
{
    if(cubemap && cubemap->get_target() != GL_TEXTURE_CUBE_MAP)
        throw std::runtime_error(
            "Given cubemap texture's target isn't GL_TEXTURE_CUBE_MAP"
        );
    this->cubemap = cubemap;
    this->pos = {pos};
}

void cubemap_visualizer_node::set_cubemap(
    texture* cubemap,
    const std::vector<vec3>& pos
){
    if(cubemap && cubemap->get_target() != GL_TEXTURE_CUBE_MAP_ARRAY)
        throw std::runtime_error(
            "Given cubemap array texture's target isn't "
            "GL_TEXTURE_CUBE_MAP_ARRAY"
        );
    this->cubemap = cubemap;
    this->pos = pos;
}

texture* cubemap_visualizer_node::get_cubemap() const
{
    return cubemap;
}

const std::vector<vec3>& cubemap_visualizer_node::get_pos() const
{
    return pos;
}

void cubemap_visualizer_node::set_radius(float radius)
{
    this->radius = radius;
}

float cubemap_visualizer_node::get_radius() const
{
    return radius;
}

void cubemap_visualizer_node::set_first_index(unsigned first_index)
{
    this->first_index = first_index;
}

unsigned cubemap_visualizer_node::get_first_index() const
{
    return first_index;
}

cubemap_visualizer_scene::cubemap_visualizer_scene() {}

void cubemap_visualizer_scene::add_cubemap_visualizer(
    cubemap_visualizer_node* visualizer
){
    sorted_insert(cubemap_visualizers, visualizer);
}

void cubemap_visualizer_scene::remove_cubemap_visualizer(
    cubemap_visualizer_node* visualizer
){
    sorted_erase(cubemap_visualizers, visualizer);
}

const std::vector<cubemap_visualizer_node*>&
cubemap_visualizer_scene::get_cubemap_visualizers() const
{
    return cubemap_visualizers;
}

void cubemap_visualizer_scene::clear_cubemap_visualizers()
{
    cubemap_visualizers.clear();
}

}

namespace lt::method
{

visualize_cubemap::visualize_cubemap(
    render_target& target,
    resource_pool& pool,
    Scene scene
):  target_method(target),
    scene_method(scene),
    visualize_shader(pool.get_shader(
        shader::path{"generic.vert", "visualize_cubemap.frag"}
    )),
    sphere(common::ensure_patched_sphere_primitive(pool, 20)),
    linear_sampler(common::ensure_linear_sampler(pool))
{}

void visualize_cubemap::execute()
{
    target_method::execute();

    if(!visualize_shader || !has_all_scenes()) return;

    const std::vector<cubemap_visualizer_node*>& cubemaps =
        get_scene<cubemap_visualizer_scene>()->get_cubemap_visualizers();

    if(cubemaps.size() == 0) return;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glFrontFace(GL_CW);

    glm::mat4 vp =
        cam->get_projection() * glm::inverse(cam->get_global_transform());

    for(cubemap_visualizer_node* n: cubemaps)
    {
        texture* cubemap = n->get_cubemap();
        if(!cubemap) continue;

        bool is_array = cubemap->get_target() == GL_TEXTURE_CUBE_MAP_ARRAY;

        shader::definition_map def;
        if(is_array) def["CUBEMAP_ARRAY"];

        shader* s = visualize_shader->get(def);
        s->bind();
        s->set("cubemap", linear_sampler.bind(*cubemap, 0));
        s->set("m", mat4(1.0f));

        const std::vector<vec3>& pos = n->get_pos();

        int max_index = min(
            (int)pos.size(),
            max((int)cubemap->get_dimensions().z, 1) - (int)n->get_first_index()
        );

        for(int i = 0; i < max_index; ++i)
        {
            mat4 m = glm::translate(pos[i]) * glm::scale(vec3(n->get_radius()));
            mat4 mvp = vp * m;
            s->set<int>("array_index", i + n->get_first_index());
            s->set("mvp", mvp);
            sphere.draw();
        }
    }

    glFrontFace(GL_CCW);
}

} // namespace lt::method
