/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_METHOD_VISUALIZE_CUBEMAP_HH
#define LT_METHOD_VISUALIZE_CUBEMAP_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../shader.hh"
#include "../sampler.hh"
#include "../scene.hh"

namespace lt
{

class texture;
class resource_pool;
class multishader;

class LT_API cubemap_visualizer_node
{
public:
    cubemap_visualizer_node(
        texture* cubemap = nullptr,
        vec3 pos = vec3(0),
        float radius = 1.0f
    );

    cubemap_visualizer_node(
        texture* cubemap_array,
        const std::vector<vec3>& pos,
        float radius = 1.0f,
        unsigned first_index = 0
    );

    void set_cubemap(texture* cubemap, vec3 pos);
    void set_cubemap(texture* cubemap, const std::vector<vec3>& pos);

    texture* get_cubemap() const;
    const std::vector<vec3>& get_pos() const;

    void set_radius(float radius);
    float get_radius() const;

    void set_first_index(unsigned first_index);
    unsigned get_first_index() const;

private:
    texture* cubemap;
    std::vector<vec3> pos;
    float radius;
    unsigned first_index;
};

class LT_API cubemap_visualizer_scene
{
public:
    cubemap_visualizer_scene();

    void add_cubemap_visualizer(cubemap_visualizer_node* visualizer);
    void remove_cubemap_visualizer(cubemap_visualizer_node* visualizer);

    const std::vector<cubemap_visualizer_node*>&
    get_cubemap_visualizers() const;

    void clear_cubemap_visualizers();

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(cubemap_visualizer_node* visualizer);
    void remove_impl(cubemap_visualizer_node* visualizer);
    void clear_impl();

private:
    std::vector<cubemap_visualizer_node*> cubemap_visualizers;
};

}

namespace lt::method
{

class LT_API visualize_cubemap:
    public target_method,
    public scene_method<camera_scene, cubemap_visualizer_scene>
{
public:
    visualize_cubemap(render_target& target, resource_pool& pool, Scene scene);

    void execute() override;

private:
    multishader* visualize_shader;
    const primitive& sphere;
    const sampler& linear_sampler;        
};

} // namespace lt::method

#endif
