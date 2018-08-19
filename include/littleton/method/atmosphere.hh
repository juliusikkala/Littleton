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
#ifndef LT_METHOD_ATMOSPHERE_HH
#define LT_METHOD_ATMOSPHERE_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../transformable.hh"
#include "../primitive.hh"
#include "../scene.hh"
#include "../sampler.hh"

namespace lt
{

class texture;
class resource_pool;
class directional_light;

struct LT_API atmosphere: public transformable_node
{
    // Default settings are for an earth-like atmosphere
    atmosphere(
        vec3 origin = vec3(0, -6.3781e6, 0),
        double ground_radius = 6.3781e6,
        double atmosphere_height = 1.0e5,
        double pressure = 1.01325e5,
        double temperature = 288.15,
        double ior = 1.000305,
        double mie_coef = 21e-6,
        double mie_anisotropy = 0.76,
        double rayleigh_scale_height = 8.5e3,
        double mie_scale_height = 1.2e3,
        float intensity = 1
    );

    void set_intensity(float intensity);

    void set_radius(double ground_radius, double atmosphere_height);

    // Assumes the atmospheric gas is air
    void set_conditions(double pressure, double temperature);

    void set_conditions(
        double pressure,
        double temperature,
        double ior,
        double mie_coef,
        double mie_anisotropy
    );

    void set_scale_height(
        double rayleigh_scale_height,
        double mie_scale_height
    );

    // Despite all these setters, direct access to all the variables is allowed.
    float intensity;
    double ground_radius, atmosphere_height;
    double rayleigh_scale_height, mie_scale_height;
    vec3 rayleigh_coef;
    double mie_coef, mie_anisotropy;
};

class LT_API atmosphere_scene
{
public:
    explicit atmosphere_scene(std::vector<atmosphere*>&& atmospheres = {});
    ~atmosphere_scene();

    void add_atmosphere(atmosphere* a);
    void remove_atmosphere(atmosphere* a);
    void clear_atmospheres();
    size_t atmosphere_count() const;

    void set_atmospheres(const std::vector<atmosphere*>& atmospheres);
    const std::vector<atmosphere*>& get_atmospheres() const;

    void set_atmosphere_sun(directional_light* sun);
    // Assumed to be infinitely far away
    void set_atmosphere_sun(point_light* sun);

    vec3 get_sun_dir(vec3 target) const;
    vec3 get_sun_color(vec3 target) const;
    vec3 get_attenuated_sun_color(
        atmosphere* a,
        vec3 pos,
        unsigned view_samples = 32
    ) const;

private:
    std::vector<atmosphere*> atmospheres;
    directional_light* directional_sun;
    point_light* point_sun;
};

}

namespace lt::method
{

LT_OPTIONS(render_atmosphere)
{
    unsigned view_samples = 10;
    unsigned light_samples = 3;
};

class LT_API render_atmosphere:
    public target_method,
    public scene_method<camera_scene, atmosphere_scene>,
    public options_method<render_atmosphere>
{
public:
    render_atmosphere(
        render_target& target,
        resource_pool& pool,
        Scene scene,
        texture* depth_buffer = nullptr,
        const options& opt = {}
    );

    void execute() override;

private:
    shader* atmosphere_shader;
    texture* depth_buffer;
    const sampler& depth_sampler;
    const primitive& quad;

    unsigned view_samples, light_samples;
};

} // namespace lt::method

#endif
