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
#ifndef LT_METHOD_VISUALIZE_GBUFFER_HH
#define LT_METHOD_VISUALIZE_GBUFFER_HH
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../shader.hh"
#include "../sampler.hh"

namespace lt
{

class gbuffer;
class resource_pool;
class render_scene;
class multishader;

}

namespace lt::method
{

class visualize_gbuffer: public target_method
{
public:
    visualize_gbuffer(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        render_scene* scene
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    enum visualizer
    {
        DEPTH,
        POSITION,
        NORMAL,
        COLOR,
        ROUGHNESS,
        METALLIC,
        IOR,
        MATERIAL
    };

    void show(visualizer full);
    void show(
        visualizer topleft,
        visualizer topright,
        visualizer bottomleft,
        visualizer bottomright
    );

    void execute() override;

    std::string get_name() const override;

private:
    gbuffer* buf;

    multishader* visualize_shader;
    render_scene* scene;
    const primitive& quad;
    const sampler& fb_sampler;        

    std::vector<visualizer> visualizers;
};

} // namespace lt::method
#endif
