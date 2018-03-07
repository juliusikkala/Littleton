#ifndef METHOD_DRAW_TEXTURE_HH
#define METHOD_DRAW_TEXTURE_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"

class texture;
class shader_pool;

namespace method
{
    class draw_texture: public target_method
    {
    public:
        draw_texture(
            render_target& target,
            shader_pool& shaders,
            texture* tex = nullptr
        );
        ~draw_texture();

        void set_transform(glm::mat4 transform);

        void set_texture(texture* tex = nullptr);

        void execute() override;

        std::string get_name() const override;

    private:
        vertex_buffer quad;

        shader* draw_shader;
        glm::mat4 transform;
        texture* tex;
    };
}

#endif

