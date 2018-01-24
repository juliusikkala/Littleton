#ifndef METHOD_FULLSCREEN_EFFECT_HH
#define METHOD_FULLSCREEN_EFFECT_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "shader.hh"

namespace method
{
    // Assumes the vertex shader is data/shaders/fullscreen.vert
    class fullscreen_effect: public pipeline_method
    {
    public:
        fullscreen_effect(shader* effect = nullptr);
        ~fullscreen_effect();

        void execute() override;

        void set_shader(shader* effect);
        shader* get_shader() const;

    private:
        shader* effect;
        vertex_buffer fullscreen_quad;
    };
}

#endif
