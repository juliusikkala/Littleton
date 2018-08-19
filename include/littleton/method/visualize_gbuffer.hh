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
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../shader.hh"
#include "../scene.hh"
#include "../sampler.hh"

namespace lt
{

class gbuffer;
class resource_pool;
class multishader;

}

namespace lt::method
{

LT_OPTIONS(visualize_gbuffer)
{
    enum visualizer
    {
        DEPTH,
        POSITION,
        NORMAL,
        COLOR,
        ROUGHNESS,
        METALLIC,
        IOR,
        MATERIAL,
        LIGHTING,
        INDIRECT_LIGHTING,
    };

    method_options();
    method_options(visualizer full);
    method_options(
        visualizer topleft,
        visualizer topright,
        visualizer bottomleft,
        visualizer bottomright
    );

    std::vector<visualizer> visualizers;
};

class LT_API visualize_gbuffer:
    public target_method,
    public scene_method<camera_scene>,
    public options_method<visualize_gbuffer>
{
public:
    visualize_gbuffer(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt
    );

    void execute() override;

private:
    gbuffer* buf;

    multishader* visualize_shader;
    const primitive& quad;
    const sampler& fb_sampler;        
};

} // namespace lt::method
#endif
