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
#ifndef LT_METHOD_VISUALIZE_CUBEMAP_HH
#define LT_METHOD_VISUALIZE_CUBEMAP_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../shader.hh"
#include "../sampler.hh"

namespace lt
{

class texture;
class resource_pool;
class render_scene;
class multishader;

}

namespace lt::method
{

class LT_API visualize_cubemap: public target_method
{
public:
    struct node
    {
        node(texture* cubemap, vec3 pos, float radius = 1.0f);
        node(
            texture* cubemap_array,
            const std::vector<vec3>& pos,
            float radius = 1.0f,
            unsigned first_index = 0
        );

        texture* cubemap;
        std::vector<vec3> pos;
        float radius;
        unsigned first_index;
    };

    visualize_cubemap(
        render_target& target,
        resource_pool& pool,
        render_scene* scene,
        const std::vector<node>& cubemaps = {}
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    void set_cubemaps(const std::vector<node>& cubemaps);
    std::vector<node>& get_cubemaps();
    const std::vector<node>& get_cubemaps() const;

    void execute() override;

    std::string get_name() const override;

private:
    multishader* visualize_shader;
    render_scene* scene;
    const primitive& sphere;
    const sampler& linear_sampler;        

    std::vector<node> cubemaps;
};

} // namespace lt::method

#endif
