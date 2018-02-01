#ifndef METHOD_FULLSCREEN_EFFECT_HH
#define METHOD_FULLSCREEN_EFFECT_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "shader.hh"
#include "texture.hh"
#include <map>
#include <string>

namespace method
{
    // Assumes the vertex shader is data/shaders/fullscreen.vert
    // Textures will be passed by their map key as the uniform name.
    class fullscreen_effect: public pipeline_method
    {
    public:
        fullscreen_effect(
            render_target& target,
            shader* effect = nullptr,
            std::map<std::string, texture*>&& textures = {}
        );
        ~fullscreen_effect();

        void execute() override;

        void set_shader(shader* effect);
        shader* get_shader() const;

        void set_texture(const std::string& name, texture* tex = nullptr);
        texture* get_texture(const std::string& name) const;
        void clear_textures();

    private:
        shader* effect;
        std::map<std::string, texture*> textures;
        vertex_buffer fullscreen_quad;
    };
}

#endif
