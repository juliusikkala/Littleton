#ifndef METHOD_TONEMAP_HH
#define METHOD_TONEMAP_HH
#include "pipeline.hh"
#include "primitive.hh"
#include "sampler.hh"

class texture;
class resource_pool;

namespace method
{
    class tonemap: public target_method
    {
    public:
        tonemap(
            render_target& target,
            resource_pool& pool,
            texture* src,
            float exposure = 1.0f
        );

        void set_exposure(float exposure);
        float get_exposure() const;

        void execute() override;

        std::string get_name() const override;

    private:
        texture* src;
        shader* tonemap_shader;
        const primitive& quad;
        const sampler& fb_sampler;

        float exposure;
    };
}
#endif
