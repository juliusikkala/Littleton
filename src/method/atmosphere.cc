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
#include "atmosphere.hh"
#include "texture.hh"
#include "math.hh"
#include "camera.hh"
#include "texture.hh"
#include "render_target.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "light.hh"
#include "scene.hh"
#include "helpers.hh"
#include <algorithm>

namespace lt
{

atmosphere::atmosphere(
    vec3 origin,
    double ground_radius,
    double atmosphere_height,
    double pressure,
    double temperature,
    double ior,
    double mie_coef,
    double mie_anisotropy,
    double rayleigh_scale_height,
    double mie_scale_height,
    float intensity
){
    set_position(origin);
    set_intensity(intensity);
    set_radius(ground_radius, atmosphere_height);
    set_conditions(pressure, temperature, ior, mie_coef, mie_anisotropy);
    set_scale_height(rayleigh_scale_height, mie_scale_height);
}

void atmosphere::set_intensity(float intensity)
{
    this->intensity = intensity;
}

void atmosphere::set_radius(double ground_radius, double atmosphere_height)
{
    this->ground_radius = ground_radius;
    this->atmosphere_height = atmosphere_height;
}

void atmosphere::set_conditions(double pressure, double temperature)
{
    set_conditions(
        pressure,
        temperature,
        1.0 + 0.000293*(pressure/1.01325e5)*(300/temperature),
        21e-6,
        0.76
    );
}

void atmosphere::set_conditions(
    double pressure,
    double temperature,
    double ior,
    double mie_coef,
    double mie_anisotropy
){
    constexpr double Na = 6.02214086e23;
    constexpr double R = 8.3144598;
    double N = (Na * pressure)/(R * temperature);

    const glm::dvec3 wavelengths = glm::pow(
        glm::dvec3(680e-9, 550e-9, 440e-9),
        glm::dvec3(4)
    );

    rayleigh_coef = (8 * pow(glm::pi<double>(), 3) * pow(pow(ior, 2) - 1, 2)) /
        (3 * N  *wavelengths);
    this->mie_coef = mie_coef;
    this->mie_anisotropy = mie_anisotropy;
}

void atmosphere::set_scale_height(
    double rayleigh_scale_height,
    double mie_scale_height
){
    this->rayleigh_scale_height = rayleigh_scale_height;
    this->mie_scale_height = mie_scale_height;
}

atmosphere_scene::atmosphere_scene(std::vector<atmosphere*>&& atmospheres)
: atmospheres(atmospheres), directional_sun(nullptr), point_sun(nullptr) {}
atmosphere_scene::~atmosphere_scene() {}

void atmosphere_scene::add_atmosphere(atmosphere* a)
{
    sorted_insert(atmospheres, a);
}

void atmosphere_scene::remove_atmosphere(atmosphere* a)
{
    sorted_erase(atmospheres, a);
}

void atmosphere_scene::clear_atmospheres()
{
    atmospheres.clear();
}

size_t atmosphere_scene::atmosphere_count() const
{
    return atmospheres.size();
}

void atmosphere_scene::set_atmospheres(
    const std::vector<atmosphere*>& atmospheres
){
    this->atmospheres = atmospheres;
}

const std::vector<atmosphere*>& atmosphere_scene::get_atmospheres() const
{
    return atmospheres;
}

void atmosphere_scene::set_atmosphere_sun(directional_light* sun)
{
    directional_sun = sun;
    point_sun = nullptr;
}

void atmosphere_scene::set_atmosphere_sun(point_light* sun)
{
    point_sun = sun;
    directional_sun = nullptr;
}

vec3 atmosphere_scene::get_sun_dir(vec3 target) const
{
    if(directional_sun) return directional_sun->get_direction();
    if(point_sun) return target - point_sun->get_global_position();
    return vec3(0);
}

vec3 atmosphere_scene::get_sun_color(vec3 target) const
{
    if(directional_sun) return directional_sun->get_color();
    if(point_sun)
    {
        return point_sun->get_color() /
            distance2(target, point_sun->get_global_position());
    }
    return vec3(0);
}

vec3 atmosphere_scene::get_attenuated_sun_color(
    atmosphere* a,
    vec3 pos,
    unsigned view_samples
) const
{
    if(!directional_sun && !point_sun) return glm::vec3(0);

    glm::vec3 origin = a->get_global_position();
    glm::vec3 dir = glm::normalize(-get_sun_dir(origin));
    float t0, t1;
    float scale = a->get_global_scaling().x;
    if(!intersect_sphere(
        pos,
        dir,
        origin,
        (float)(scale*(a->ground_radius + a->atmosphere_height)),
        t0,
        t1
    )) return glm::vec3(0);

    glm::vec3 x0 = pos + t0 * dir;
    float segment_length = (t1 - t0) / view_samples;
    glm::vec3 segment = dir * segment_length;
    glm::vec3 x = x0 + segment * 0.5f;

    float r_optical_depth = 0;
    float m_optical_depth = 0;

    for(unsigned i = 0; i < view_samples; ++i)
    {
        float h = (float)(distance(x, origin) - a->ground_radius*scale);
        float r_h = (float)exp(-h / (a->rayleigh_scale_height*scale)) * segment_length;
        float m_h = (float)exp(-h / (a->mie_scale_height*scale)) * segment_length;
        r_optical_depth += r_h;
        m_optical_depth += m_h;

        x += segment;
    }

    glm::vec3 T = a->rayleigh_coef * r_optical_depth / scale +
             glm::vec3((float)(a->mie_coef * 1.1 * m_optical_depth / scale));
    glm::vec3 attenuation = glm::exp(-T);

    return attenuation * get_sun_color(origin);
}

}

namespace lt::method
{

render_atmosphere::render_atmosphere(
    render_target& target,
    resource_pool& pool,
    Scene scene,
    texture* depth_buffer,
    const options& opt
):  target_method(target),
    scene_method(scene),
    options_method(opt),
    atmosphere_shader(
        pool.get_shader(shader::path{"atmosphere.vert", "atmosphere.frag"}, {})
    ),
    depth_buffer(depth_buffer),
    depth_sampler(common::ensure_depth_sampler(pool)),
    quad(common::ensure_quad_primitive(pool))
{
}

void render_atmosphere::execute()
{
    target_method::execute();
    const auto [view_samples, light_samples] = opt;

    if(!atmosphere_shader || !depth_buffer || !has_all_scenes()) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_STENCIL_TEST);

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    atmosphere_scene* atmospheres = get_scene<atmosphere_scene>();

    // Sort atmospheres by view depth, they shouldn't ever overlap in realistic
    // situations.
    struct atmosphere_pos {
        vec3 view_pos;
        vec3 sun_dir;
        vec3 sun_color;
        atmosphere* a;
    };

    std::vector<atmosphere_pos> sorted_atmospheres;
    sorted_atmospheres.reserve(atmospheres->atmosphere_count());

    for(atmosphere* a: atmospheres->get_atmospheres())
    {
        vec3 pos = a->get_global_position();
        sorted_atmospheres.push_back({
            vec3(v * vec4(pos, 1)),
            normalize(vec3(v * vec4(atmospheres->get_sun_dir(pos), 0))),
            atmospheres->get_sun_color(pos),
            a
        });
    }

    std::sort(
        sorted_atmospheres.begin(),
        sorted_atmospheres.end(),
        [](const atmosphere_pos& a, const atmosphere_pos& b)
        {
            return a.view_pos.z < b.view_pos.z;
        }
    );

    atmosphere_shader->bind();
    atmosphere_shader->set("in_depth", depth_sampler.bind(*depth_buffer));
    atmosphere_shader->set("clip_info", cam->get_clip_info());
    atmosphere_shader->set("projection_info", cam->get_projection_info());

    // Draw all atmospheres in order
    for(atmosphere_pos& ap: sorted_atmospheres)
    {
        atmosphere* a = ap.a;
        // Can't scale this non-uniformly.
        float scale = a->get_global_scaling().x;
        atmosphere_shader->set<int>("view_samples", view_samples);
        atmosphere_shader->set<int>("light_samples", light_samples);
        atmosphere_shader->set("rayleigh_coef", a->rayleigh_coef/scale);
        atmosphere_shader->set<float>(
            "inv_rayleigh_scale_height",
            1.0f/(float)(a->rayleigh_scale_height*scale)
        );
        atmosphere_shader->set<float>(
            "inv_mie_scale_height",
            1.0f/(float)(a->mie_scale_height*scale)
        );
        atmosphere_shader->set<float>(
            "mie_coef",
            (float)(a->mie_coef/scale)
        );
        atmosphere_shader->set<float>(
            "mie_anisotropy",
            (float)a->mie_anisotropy
        );
        atmosphere_shader->set<float>(
            "ground_radius",
            (float)(scale * a->ground_radius)
        );
        atmosphere_shader->set<float>(
            "atmosphere_radius2",
            powf((float)(scale*(a->ground_radius + a->atmosphere_height)), 2.0f)
        );
        atmosphere_shader->set("sun_direction", -ap.sun_dir);
        atmosphere_shader->set("sun_color", a->intensity * ap.sun_color);
        atmosphere_shader->set("ip", inverse(p));
        atmosphere_shader->set("origin", ap.view_pos);
        quad.draw();
    }
}

std::string render_atmosphere::get_name() const
{
    return "render_atmosphere";
}

} // namespace lt::method
