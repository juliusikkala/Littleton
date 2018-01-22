#ifndef METHOD_FULLSCREEN_EFFECT_HH
#define METHOD_FULLSCREEN_EFFECT_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "shader.hh"

namespace method
{
    class fullscreen_effect: public pipeline_method
    {
    public:
        fullscreen_effect(const std::string& fshader_source);
        ~fullscreen_effect();

        void execute() override;

        shader* get_shader();
        const shader* get_shader() const;

    private:
        shader effect;
        vertex_buffer fullscreen_quad;
    };
}

#endif
