#include "sky.hh"
#include "texture.hh"
#include <glm/gtc/constants.hpp>

static struct sky_defaults
{
    sky_defaults()
    {
        parent.set_position(glm::vec3(0, -6.3781e6, 0));
    }

    transformable_node parent;
} defaults;

method::sky::sky(
    render_target& target,
    shader_store& store,
    render_scene* scene,
    texture* depth_buffer,
    directional_light* sun
): target_method(target),
   sky_shader(store.get(shader::path{"sky.vert", "sky.frag"})),
   scene(scene),
   depth_buffer(depth_buffer),
   fullscreen_quad(vertex_buffer::create_square(target.get_context())),
   sun(sun)
{
    set_parent(&defaults.parent);
    set_radius();
    set_conditions();
    set_scale_height();
}

void method::sky::set_scene(render_scene* s)
{
    scene = s;
}

render_scene* method::sky::get_scene() const
{
    return scene;
}

void method::sky::set_parent(transformable_node* parent)
{
    origin_node.set_parent(parent);
}

void method::sky::set_origin(glm::vec3 origin)
{
    origin_node.set_position(origin);
}

void method::sky::set_sun(directional_light* sun)
{
    this->sun = sun;
}

void method::sky::set_radius(double ground_radius, double atmosphere_height)
{
    this->ground_radius = ground_radius;
    this->atmosphere_height = atmosphere_height;
}

void method::sky::set_conditions(double pressure, double temperature)
{
    set_conditions(
        pressure,
        temperature,
        1.0 + 0.000293*(pressure/1.01325e5)*(300/temperature)
    );
}

void method::sky::set_conditions(
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

void method::sky::set_scale_height(
    double rayleigh_scale_height,
    double mie_scale_height
){
    this->rayleigh_scale_height = rayleigh_scale_height;
    this->mie_scale_height = mie_scale_height;
}

void method::sky::execute()
{
    target_method::execute();

    if(!sky_shader || !depth_buffer || !scene || !sun) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_STENCIL_TEST);

    camera* cam = scene->get_camera();
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
    float near, far, fov, aspect;
    decompose_perspective(p, near, far, fov, aspect);
    glm::vec4 perspective_data = glm::vec4(
        2*tan(fov/2.0f)*aspect,
        2*tan(fov/2.0f),
        near,
        far
    );

    shader* s = sky_shader->get({
        {"VIEW_SAMPLES", "16"},
        {"LIGHT_SAMPLES", "3"}
    });

    s->bind();
    s->set("rayleigh_coef", rayleigh_coef);
    s->set<float>("inv_rayleigh_scale_height", 1/rayleigh_scale_height);
    s->set<float>("inv_mie_scale_height", 1/mie_scale_height);
    s->set<float>("mie_coef", mie_coef);
    s->set<float>("mie_anisotropy", mie_anisotropy);
    s->set<float>("ground_radius", ground_radius);
    s->set<float>(
        "atmosphere_radius2",
        pow(ground_radius + atmosphere_height, 2)
    );
    s->set("in_depth", depth_buffer->bind());
    s->set("sun_direction", -sun_direction);
    s->set("sun_color", sun_color);
    s->set("ip", glm::inverse(p));
    s->set("origin", origin);
    s->set("perspective_data", perspective_data);
    fullscreen_quad.draw();
}
