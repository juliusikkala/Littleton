#ifndef METHOD_SKY_HH
#define METHOD_SKY_HH
#include "pipeline.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"
#include "scene.hh"

namespace method
{
    class sky: public target_method
    {
    public:
        sky(
            render_target& target,
            shader_store& shaders,
            render_scene* scene = nullptr,
            texture* depth_buffer = nullptr,
            directional_light* sun = nullptr
        );

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void set_parent(transformable_node* parent = nullptr);
        void set_origin(glm::vec3 origin = glm::vec3(0));

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
            double rayleigh_scale_height = 8.5e3,
            double mie_scale_height = 1.2e3,
            double mie_coef = 210e-5,
            double mie_anisotropy = 0.76
        );

        void execute() override;

    private:
        multishader* sky_shader;
        render_scene* scene;
        texture* depth_buffer;
        vertex_buffer fullscreen_quad;

        transformable_node origin_node;
        double ground_radius, atmosphere_height;
        double rayleigh_scale_height, mie_scale_height;
        glm::vec3 rayleigh_coef;
        double mie_coef, mie_anisotropy;
        directional_light* sun;
    };
}
#endif
