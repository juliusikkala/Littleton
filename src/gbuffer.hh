#ifndef LT_GBUFFER_HH
#define LT_GBUFFER_HH
#include "texture.hh"
#include "shader.hh"
#include "render_target.hh"
#include "math.hh"

namespace lt
{

class sampler;
class shader_pool;
class primitive;

class gbuffer: public render_target
{
public:
    // If depth_stencil isn't provided, an RBO is created for it.
    gbuffer(
        context& ctx,
        glm::uvec2 size,
        texture* normal = nullptr,
        texture* color = nullptr,
        texture* material = nullptr,
        texture* lighting = nullptr,
        texture* linear_depth = nullptr,
        texture* depth_stencil = nullptr
    );
    gbuffer(gbuffer&& other);
    ~gbuffer();

    texture* get_normal() const;
    texture* get_color() const;
    texture* get_material() const;
    texture* get_linear_depth() const;
    texture* get_lighting() const;
    texture* get_depth_stencil() const;

    // -1 if not bound on drawBuffers
    int get_normal_index() const;
    int get_color_index() const;
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

    shader* get_min_max_shader(shader_pool& pool) const;
    // Warning: modifies OpenGL state variables
    void render_depth_mipmaps(
        shader* min_max,
        const primitive& quad,
        const sampler& fb_sampler
    );

private:
    draw_mode mode;

    texture* normal;
    texture* color;
    texture* material;//roughness, metallic, ior
    texture* linear_depth;
    texture* lighting;
    texture* depth_stencil;
    GLuint depth_stencil_rbo;

    int normal_index;
    int color_index;
    int material_index;
    int linear_depth_index;
    int lighting_index;
};

} // namespace lt

#endif
