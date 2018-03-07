#ifndef METHOD_KERNEL_HH
#define METHOD_KERNEL_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"

class texture;
class shader_pool;

namespace method
{
    class kernel: public target_method
    {
    public:
        static const glm::mat3 SHARPEN;
        static const glm::mat3 EDGE_DETECT;
        static const glm::mat3 GAUSSIAN_BLUR;
        static const glm::mat3 BOX_BLUR;

        kernel(
            render_target& target,
            texture& src,
            shader_pool& store,
            const glm::mat3& k = SHARPEN
        );

        void set_kernel(const glm::mat3& kernel);
        glm::mat3 get_kernel() const;

        void execute() override;

        std::string get_name() const override;

    private:
        texture* src;
        shader* kernel_shader;
        vertex_buffer fullscreen_quad;
        glm::mat3 k;
    };
}
#endif
