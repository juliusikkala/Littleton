#ifndef GBUFFER_HH
#define GBUFFER_HH
#include "texture.hh"
#include "shader.hh"
#include "render_target.hh"
#include <glm/glm.hpp>

class sampler;

class gbuffer: public render_target
{
public:
    // If depth_stencil isn't provided, an RBO is created for it.
    gbuffer(
        context& ctx,
        glm::uvec2 size,
        texture* normal = nullptr,
        texture* color_emission = nullptr,
        texture* material = nullptr,
        texture* lighting = nullptr,
        texture* linear_depth = nullptr,
        texture* depth_stencil = nullptr
    );
    gbuffer(gbuffer&& other);
    ~gbuffer();

    texture* get_normal() const;
    texture* get_color_emission() const;
    texture* get_material() const;
    texture* get_linear_depth() const;
    texture* get_lighting() const;
    texture* get_depth_stencil() const;

    // -1 if not bound on drawBuffers
    int get_normal_index() const;
    int get_color_emission_index() const;
    int get_material_index() const;
    int get_linear_depth_index() const;
    int get_lighting_index() const;

    void bind_textures(const sampler& fb_sampler) const;
    // Call bind_textures at least once before this!
    void set_uniforms(shader* s) const;
    void update_definitions(shader::definition_map& def) const;

    // Assumes either draw_all or draw_geometry is in effect
    void clear();

    enum draw_mode
    {
        DRAW_ALL = 0,
        DRAW_GEOMETRY = 1,
        DRAW_LIGHTING = 2
    };

    void set_draw(draw_mode mode);
    draw_mode get_draw() const;

private:
    draw_mode mode;

    texture* normal;
    texture* color_emission;
    texture* material;//roughness, metallic, ior, subsurface depth
    //texture* subsurface;
    texture* linear_depth;
    texture* lighting;
    texture* depth_stencil;
    GLuint depth_stencil_rbo;

    int normal_index;
    int color_emission_index;
    int material_index;
    int linear_depth_index;
    int lighting_index;
};

#endif
