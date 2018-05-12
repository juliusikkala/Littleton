#ifndef METHOD_FULLSCREEN_EFFECT_HH
#define METHOD_FULLSCREEN_EFFECT_HH
#include "pipeline.hh"
#include "primitive.hh"
#include <map>
#include <string>

class texture;
class shader;
class resource_pool;

namespace method
{
    // Assumes the vertex shader is data/shaders/fullscreen.vert
    class fullscreen_effect: public target_method
    {
    public:
        fullscreen_effect(
            render_target& target,
            resource_pool& pool,
            shader* effect = nullptr
        );
        ~fullscreen_effect();

        void execute() override;

        void set_shader(shader* effect);
        shader* get_shader() const;

        std::string get_name() const override;

    private:
        shader* effect;
        const primitive& quad;
    };
}

#endif
