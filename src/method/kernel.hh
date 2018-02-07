#ifndef METHOD_KERNEL_HH
#define METHOD_KERNEL_HH
#include "pipeline.hh"
#include "texture.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"

namespace method
{
    class kernel: public pipeline_method
    {
    public:
        static const glm::mat3 SHARPEN;
        static const glm::mat3 EDGE_DETECT;
        static const glm::mat3 GAUSSIAN_BLUR;
        static const glm::mat3 BOX_BLUR;

        kernel(
            render_target& target,
            texture& src,
            shader_store& store,
            const glm::mat3& k = SHARPEN
        );

        void set_kernel(const glm::mat3& kernel);
        glm::mat3 get_kernel() const;

        void execute() override;

    private:
        texture* src;
        shader* kernel_shader;
        vertex_buffer fullscreen_quad;
        glm::mat3 k;
    };
}
#endif
