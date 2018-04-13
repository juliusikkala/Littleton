#include "gbuffer.hh"
#include "sampler.hh"
#include "shader.hh"
#include "shader_pool.hh"
#include "vertex_buffer.hh"
#include <stdexcept>
#include <cmath>

gbuffer::gbuffer(
    context& ctx,
    glm::uvec2 size,
    texture* normal,
    texture* color_emission,
    texture* material,
    texture* lighting,
    texture* linear_depth,
    texture* depth_stencil
):  render_target(ctx, size),
    normal(normal), color_emission(color_emission), material(material),
    linear_depth(linear_depth), lighting(lighting),
    depth_stencil(depth_stencil), depth_stencil_rbo(0)
{
    // Validate textures first
    if(
        normal != nullptr && (
        normal->get_size() != size ||
        normal->get_external_format() != GL_RG)
    ) throw std::runtime_error("Incompatible normal");

    if(
        color_emission != nullptr && (
        color_emission->get_size() != size ||
        color_emission->get_external_format() != GL_RGBA)
    ) throw std::runtime_error("Incompatible color_emission");

    if(
        material != nullptr && (
        material->get_size() != size ||
        material->get_external_format() != GL_RGBA)
    ) throw std::runtime_error("Incompatible material");

    if(lighting != nullptr && lighting->get_size() != size)
        throw std::runtime_error("Incompatible lighting");

    if(linear_depth != nullptr && linear_depth->get_size() != size)
        throw std::runtime_error("Incompatible linear_depth");

    if(
        depth_stencil != nullptr && (
        depth_stencil->get_size() != size ||
        depth_stencil->get_internal_format() != GL_DEPTH24_STENCIL8)
    ) throw std::runtime_error("Incompatible depth_stencil");

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(depth_stencil)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            depth_stencil->get_target(),
            depth_stencil->get_texture(),
            0
        );
    }
    else
    {
        glGenRenderbuffers(1, &depth_stencil_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            size.x,
            size.y
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_stencil_rbo
        );
    }

    if(color_emission)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            color_emission->get_target(),
            color_emission->get_texture(),
            0
        );
    }

    if(normal)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT1,
            normal->get_target(),
            normal->get_texture(),
            0
        );
    }

    if(material)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT2,
            material->get_target(),
            material->get_texture(),
            0
        );
    }

    if(linear_depth)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT3,
            linear_depth->get_target(),
            linear_depth->get_texture(),
            0
        );
    }

    if(lighting)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT4,
            lighting->get_target(),
            lighting->get_texture(),
            0
        );
        glReadBuffer(GL_COLOR_ATTACHMENT4);
    }

    set_draw(DRAW_LIGHTING);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("GBuffer is incomplete!");

    reinstate_current_fbo();
}

gbuffer::gbuffer(gbuffer&& other)
:   render_target(other), mode(other.mode),
    normal(other.normal), color_emission(other.color_emission),
    material(other.material), linear_depth(other.linear_depth),
    lighting(other.lighting), depth_stencil(other.depth_stencil),
    depth_stencil_rbo(other.depth_stencil_rbo),
    normal_index(other.normal_index),
    color_emission_index(other.color_emission_index),
    material_index(other.material_index),
    linear_depth_index(other.linear_depth_index),
    lighting_index(other.lighting_index)
{
    other.fbo = 0;
    other.depth_stencil_rbo = 0;
}

gbuffer::~gbuffer()
{
    if(depth_stencil_rbo != 0) glDeleteRenderbuffers(1, &depth_stencil_rbo);
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

texture* gbuffer::get_normal() const { return normal; }
texture* gbuffer::get_color_emission() const { return color_emission; }
texture* gbuffer::get_material() const { return material; }
texture* gbuffer::get_linear_depth() const { return linear_depth; }
texture* gbuffer::get_lighting() const { return lighting; }
texture* gbuffer::get_depth_stencil() const { return depth_stencil; }

int gbuffer::get_normal_index() const { return normal_index; }
int gbuffer::get_color_emission_index() const { return color_emission_index; }
int gbuffer::get_material_index() const { return material_index; }
int gbuffer::get_linear_depth_index() const { return linear_depth_index; }
int gbuffer::get_lighting_index() const { return lighting_index; }

void gbuffer::bind_textures(const sampler& fb_sampler) const
{
    if(depth_stencil) fb_sampler.bind(*depth_stencil, 0);
    if(color_emission) fb_sampler.bind(*color_emission, 1);
    if(normal) fb_sampler.bind(*normal, 2);
    if(material) fb_sampler.bind(*material, 3);
    if(linear_depth) fb_sampler.bind(*linear_depth, 4);
    if(lighting) fb_sampler.bind(*lighting, 5);
}

void gbuffer::set_uniforms(shader* s) const
{
    if(depth_stencil) s->set("in_depth", 0);
    if(color_emission) s->set("in_color_emission", 1);
    if(normal) s->set("in_normal", 2);
    if(material) s->set("in_material", 3);
    if(linear_depth) s->set("in_linear_depth", 4);
    if(lighting) s->set("in_lighting", 5);
}

void gbuffer::update_definitions(shader::definition_map& def) const
{
    if(normal_index >= 0) def["NORMAL_INDEX"] = std::to_string(normal_index);
    else def.erase("NORMAL_INDEX");

    if(color_emission_index >= 0)
        def["COLOR_EMISSION_INDEX"] = std::to_string(color_emission_index);
    else def.erase("COLOR_EMISSION_INDEX");

    if(material_index >= 0)
        def["MATERIAL_INDEX"] = std::to_string(material_index);
    else def.erase("MATERIAL_INDEX");

    if(linear_depth_index >= 0)
    {
        def["LINEAR_DEPTH_INDEX"] = std::to_string(linear_depth_index);
        def["LINEAR_DEPTH_TYPE"] =
            linear_depth->get_external_format() == GL_RG ? "vec2" : "float";
    }
    else def.erase("LINEAR_DEPTH_INDEX");

    if(lighting_index >= 0)
        def["LIGHTING_INDEX"] = std::to_string(lighting_index);
    else def.erase("LIGHTING_INDEX");
}

void gbuffer::clear()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glStencilMask(0xFF);
    static const float zero[4] = {0};
    static const float neg_infinite[4] = {
        -INFINITY, -INFINITY, -INFINITY, -INFINITY
    };

    if(normal_index >= 0)
        glClearBufferfv(GL_COLOR, normal_index, zero);
    if(color_emission_index >= 0)
        glClearBufferfv(GL_COLOR, color_emission_index, zero);
    if(material_index >= 0)
        glClearBufferfv(GL_COLOR, material_index, zero);
    if(linear_depth_index >= 0)
        glClearBufferfv(GL_COLOR, linear_depth_index, neg_infinite);
    if(lighting_index >= 0)
        glClearBufferfv(GL_COLOR, lighting_index, zero);

    glClearDepth(1);
    glClearStencil(0);
    glClear(GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    reinstate_current_fbo();
}

void gbuffer::set_draw(draw_mode mode)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    std::vector<unsigned> attachments;
    int index = 0;

    switch(mode)
    {
    case DRAW_ALL:
        if(color_emission)
        {
            color_emission_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT0);
        }

        if(normal)
        {
            normal_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT1);
        }

        if(material)
        {
            material_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT2);
        }

        if(linear_depth)
        {
            linear_depth_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT3);
        }

        if(lighting)
        {
            lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT4);
        }

        break;
    case DRAW_GEOMETRY:
        if(color_emission)
        {
            color_emission_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT0);
        }

        if(normal)
        {
            normal_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT1);
        }

        if(material)
        {
            material_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT2);
        }

        if(linear_depth)
        {
            linear_depth_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT3);
        }
        lighting_index = -1;
        break;
    case DRAW_LIGHTING:
        color_emission_index = -1;
        normal_index = -1;
        material_index = -1;
        linear_depth_index = -1;
        if(lighting)
        {
            lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT4);
        }
        break;
    }

    if(index == 0)
        throw std::runtime_error("G-Buffer doesn't have requested buffers!");

    glDrawBuffers(attachments.size(), attachments.data());

    this->mode = mode;

    reinstate_current_fbo();
}

gbuffer::draw_mode gbuffer::get_draw() const
{
    return mode;
}

shader* gbuffer::get_min_max_shader(shader_pool& pool) const
{
    if(!linear_depth) return nullptr;
    if(linear_depth->get_external_format() == GL_RG)
    {
        return pool.get(
            shader::path{"fullscreen.vert", "min_max_depth.frag"},
            {{"MINIMUM", ""}, {"MAXIMUM", ""}}
        );
    }

    return pool.get(
        shader::path{"fullscreen.vert", "min_max_depth.frag"},
        {{"MAXIMUM", ""}}
    );
}

void gbuffer::render_depth_mipmaps(
    shader* min_max,
    const vertex_buffer& quad,
    const sampler& fb_sampler
){
    if(!min_max || !linear_depth) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glm::uvec2 size = get_size();
    unsigned mipmap_count = floor(log2(std::max(size.x, size.y)))+1;

    GLenum target = linear_depth->get_target();
    GLuint tex = linear_depth->get_texture();

    min_max->bind();
    min_max->set("prev", fb_sampler.bind(tex));

    glDrawBuffer(GL_COLOR_ATTACHMENT3);

    // Render mipmaps
    for(unsigned i = 1; i < mipmap_count; ++i)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT3,
            target, tex, i
        );
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, i-1);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, i-1);
        min_max->set<bool>("handle_top_edge", size.y&1);
        min_max->set<bool>("handle_right_edge", size.x&1);
        min_max->set<bool>("handle_both_edges", (size.x&1)&&(size.y&1));

        size = glm::max(size/2u, glm::uvec2(1));
        glViewport(0,0,size.x,size.y);
        quad.draw();
    }

    // Restore original state
    size = get_size();
    glViewport(0,0,size.x,size.y);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT3,
        target, tex, 0
    );

    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000);

    set_draw(mode);

    reinstate_current_fbo();
}
