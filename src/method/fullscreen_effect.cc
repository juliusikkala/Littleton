#include "fullscreen_effect.hh"
#include "helpers.hh"
#include "render_target.hh"
#include "shader.hh"
#include "texture.hh"
#include "common_resources.hh"

method::fullscreen_effect::fullscreen_effect(
    render_target& target,
    resource_pool& pool,
    shader* effect,
    std::map<std::string, texture*>&& textures
): target_method(target), effect(effect), textures(std::move(textures)),
   quad(common::ensure_quad_vertex_buffer(pool))
{}

method::fullscreen_effect::~fullscreen_effect() { }

void method::fullscreen_effect::execute()
{
    target_method::execute();

    if(!effect) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    effect->bind();

    int i = 0;
    for(auto& pair: textures)
    {
        pair.second->bind(i);
        effect->set(pair.first, i);
    }

    quad.draw();
}

void method::fullscreen_effect::set_shader(shader* effect)
{
    this->effect = effect;
}

shader* method::fullscreen_effect::get_shader() const
{
    return effect;
}

void method::fullscreen_effect::set_texture(
    const std::string& name, texture* tex
){
    if(tex) textures[name] = tex;
    else textures.erase(name);
}

texture* method::fullscreen_effect::get_texture(const std::string& name) const
{
    auto it = textures.find(name);
    if(it == textures.end()) return nullptr;
    return it->second;
}

void method::fullscreen_effect::clear_textures()
{
    textures.clear();
}

std::string method::fullscreen_effect::get_name() const
{
    return "fullscreen_effect";
}
