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
#include "sky.hh"
#include "texture.hh"
#include "math.hh"
#include "camera.hh"
#include "texture.hh"
#include "render_target.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "light.hh"
#include "scene.hh"

namespace
{
using namespace lt;

struct sky_defaults
{
    sky_defaults()
    {
        parent.set_position(glm::vec3(0, -6.3781e6, 0));
    }

    transformable_node parent;
} defaults;

}

namespace lt::method
{

sky::sky(
    render_target& target,
    resource_pool& pool,
    Scene scene,
    texture* depth_buffer,
    directional_light* sun
):  target_method(target),
    scene_method(scene),
    sky_shader(pool.get_shader(shader::path{"sky.vert", "sky.frag"}, {})),
    depth_buffer(depth_buffer),
    depth_sampler(common::ensure_depth_sampler(pool)),
    quad(common::ensure_quad_primitive(pool)),
    sun(sun)
{
    set_parent(&defaults.parent);
    set_samples();
    set_intensity();
    set_radius();
    set_conditions();
    set_scale_height();
}

void sky::set_parent(transformable_node* parent)
{
    origin_node.set_parent(parent);
}

void sky::set_origin(glm::vec3 origin)
{
    origin_node.set_position(origin);
}

void sky::set_scaling(float scale)
{
    origin_node.set_scaling(glm::vec3(scale));
}

void sky::set_samples(unsigned view_samples, unsigned light_samples)
{
    this->view_samples = view_samples;
    this->light_samples = light_samples;
}

void sky::set_intensity(float intensity)
{
    this->intensity = intensity;
}

void sky::set_sun(directional_light* sun)
{
    this->sun = sun;
}

glm::vec3 sky::get_attenuated_sun_color(glm::vec3 pos)
{
    if(!sun) return glm::vec3(0);

    glm::vec3 dir = glm::normalize(-sun->get_direction());
    glm::vec3 origin = origin_node.get_global_position();
    float t0, t1;
    float scale = origin_node.get_global_scaling().x;
    if(!intersect_sphere(
        pos,
        dir,
        origin,
        (float)(scale*(ground_radius + atmosphere_height)),
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
        float h = (float)(glm::distance(x, origin) - ground_radius*scale);
        float r_h = (float)exp(-h / (rayleigh_scale_height*scale)) * segment_length;
        float m_h = (float)exp(-h / (mie_scale_height*scale)) * segment_length;
        r_optical_depth += r_h;
        m_optical_depth += m_h;

        x += segment;
    }

    glm::vec3 T = rayleigh_coef * r_optical_depth / scale +
             glm::vec3((float)(mie_coef * 1.1 * m_optical_depth / scale));
    glm::vec3 attenuation = glm::exp(-T);

    return attenuation * sun->get_color();
}

void sky::set_radius(double ground_radius, double atmosphere_height)
{
    this->ground_radius = ground_radius;
    this->atmosphere_height = atmosphere_height;
}

void sky::set_conditions(double pressure, double temperature)
{
    set_conditions(
        pressure,
        temperature,
        1.0 + 0.000293*(pressure/1.01325e5)*(300/temperature)
    );
}

void sky::set_conditions(
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

    rayleigh_coef = (8*pow(glm::pi<double>(), 3)*pow(pow(ior, 2)-1, 2))/(3*N*wavelengths);
    this->mie_coef = mie_coef;
    this->mie_anisotropy = mie_anisotropy;
}

void sky::set_scale_height(
    double rayleigh_scale_height,
    double mie_scale_height
){
    this->rayleigh_scale_height = rayleigh_scale_height;
    this->mie_scale_height = mie_scale_height;
}

void sky::execute()
{
    target_method::execute();

    if(!sky_shader || !depth_buffer || !has_all_scenes() || !sun) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_STENCIL_TEST);

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::vec3 origin = glm::vec3(
        v * glm::vec4(origin_node.get_global_position(), 1)
    );

    glm::vec3 sun_direction = glm::normalize(glm::vec3(
        v * glm::vec4(sun->get_direction(), 0)
    ));
    glm::vec3 sun_color = sun->get_color();

    glm::mat4 p = cam->get_projection();

    // Can't scale this non-uniformly.
    float scale = origin_node.get_global_scaling().x;

    sky_shader->bind();
    sky_shader->set<int>("view_samples", view_samples);
    sky_shader->set<int>("light_samples", light_samples);
    sky_shader->set("rayleigh_coef", rayleigh_coef/scale);
    sky_shader->set<float>(
        "inv_rayleigh_scale_height",
        1.0f/(float)(rayleigh_scale_height*scale)
    );
    sky_shader->set<float>("inv_mie_scale_height", 1.0f/(float)(mie_scale_height*scale));
    sky_shader->set<float>("mie_coef", (float)(mie_coef/scale));
    sky_shader->set<float>("mie_anisotropy", (float)mie_anisotropy);
    sky_shader->set<float>("ground_radius", (float)(scale * ground_radius));
    sky_shader->set<float>(
        "atmosphere_radius2",
        powf((float)(scale*(ground_radius + atmosphere_height)), 2.0f)
    );
    sky_shader->set("in_depth", depth_sampler.bind(*depth_buffer));
    sky_shader->set("sun_direction", -sun_direction);
    sky_shader->set("sun_color", intensity * sun_color);
    sky_shader->set("ip", glm::inverse(p));
    sky_shader->set("origin", origin);
    sky_shader->set("projection_info", cam->get_projection_info());
    sky_shader->set("clip_info", cam->get_clip_info());
    quad.draw();
}

std::string sky::get_name() const
{
    return "sky";
}

} // namespace lt::method
