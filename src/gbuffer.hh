#ifndef GBUFFER_HH
#define GBUFFER_HH
#include "texture.hh"
#include "render_target.hh"
#include <glm/glm.hpp>

class gbuffer: public render_target
{
public:
    gbuffer(
        context& ctx,
        glm::uvec2 size,
        texture* lighting_target = nullptr
    );
    gbuffer(gbuffer&& other);
    ~gbuffer();

    texture& get_depth_stencil();
    texture& get_color_emission();
    texture& get_normal();
    texture& get_material();

    // Assumes either draw_all or draw_geometry is in effect
    void clear();
    void draw_all();
    void draw_geometry();
    void draw_lighting();

private:
    texture depth_stencil;
    texture color_emission;
    texture normal;
    texture material;//roughness, metallic, ior, subsurface depth
    //texture subsurface;
};

#endif
