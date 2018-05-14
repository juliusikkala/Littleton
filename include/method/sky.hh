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
#ifndef LT_METHOD_SKY_HH
#define LT_METHOD_SKY_HH
#include "pipeline.hh"
#include "transformable.hh"
#include "primitive.hh"
#include "sampler.hh"

namespace lt
{

class texture;
class resource_pool;
class render_scene;
class directional_light;

}

namespace lt::method
{

class sky: public target_method
{
public:
    sky(
        render_target& target,
        resource_pool& pool,
        render_scene* scene = nullptr,
        texture* depth_buffer = nullptr,
        directional_light* sun = nullptr
    );

    void set_scene(render_scene* s);
    render_scene* get_scene() const;

    void set_parent(transformable_node* parent = nullptr);
    void set_origin(glm::vec3 origin = glm::vec3(0));
    void set_scaling(float scale);
    void set_samples(
        unsigned view_samples = 10,
        unsigned light_samples = 3
    );
    void set_intensity(float intensity = 10);

    glm::vec3 get_attenuated_sun_color(glm::vec3 pos = glm::vec3(0));

    void set_sun(directional_light* sun = nullptr);

    void set_radius(
        double ground_radius = 6.3781e6,
        double atmosphere_height = 1.0e5
    );

    // Assumes the atmospheric gas is air
    void set_conditions(
        double pressure = 1.01325e5,
        double temperature = 288.15
    );

    void set_conditions(
        double pressure,
        double temperature,
        double ior,
        double mie_coef = 21e-6,
        double mie_anisotropy = 0.76
    );

    void set_scale_height(
        double rayleigh_scale_height = 8.5e3,
        double mie_scale_height = 1.2e3
    );

    void execute() override;

    std::string get_name() const override;

private:
    shader* sky_shader;
    render_scene* scene;
    texture* depth_buffer;
    const sampler& depth_sampler;
    const primitive& quad;

    transformable_node origin_node;
    unsigned view_samples, light_samples;
    float intensity;
    double ground_radius, atmosphere_height;
    double rayleigh_scale_height, mie_scale_height;
    glm::vec3 rayleigh_coef;
    double mie_coef, mie_anisotropy;
    directional_light* sun;
};

} // namespace lt::method

#endif
