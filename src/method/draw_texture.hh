#ifndef METHOD_DRAW_TEXTURE_HH
#define METHOD_DRAW_TEXTURE_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"

class texture;
class shader_store;

namespace method
{
    class draw_texture: public target_method
    {
    public:
        draw_texture(
            render_target& target,
            shader_store& shaders,
            texture* tex = nullptr
        );
        ~draw_texture();

        void set_transform(glm::mat4 transform);

        void set_texture(texture* tex = nullptr);

        void execute() override;

    private:
        vertex_buffer quad;

        shader* draw_shader;
        glm::mat4 transform;
        texture* tex;
    };
}

#endif

