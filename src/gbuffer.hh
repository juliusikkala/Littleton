#ifndef GBUFFER_HH
#define GBUFFER_HH
#include "texture.hh"
#include "render_target.hh"
#include <glm/glm.hpp>

class gbuffer: public render_target
{
public:
    gbuffer(context& ctx, glm::uvec2 size);
    gbuffer(gbuffer&& other);
    ~gbuffer();

    texture& get_depth_stencil();
    texture& get_color_emission();
    texture& get_normal();

private:
    texture depth_stencil;
    texture color_emission;
    texture normal;
    //texture subsurface;
    //texture material;//roughness, metallic, ior, depth
};

#endif
