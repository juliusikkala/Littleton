/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "gbuffer.hh"
#include "sampler.hh"
#include "shader.hh"
#include "shader_pool.hh"
#include "primitive.hh"
#include <stdexcept>

namespace lt
{

gbuffer::gbuffer(
    context& ctx,
    glm::uvec2 size,
    texture* normal,
    texture* color,
    texture* material,
    texture* lighting,
    texture* linear_depth,
    texture* depth_stencil,
    texture* indirect_lighting
):  render_target(ctx, GL_TEXTURE_2D, glm::uvec3(size, 1)),
    normal(normal), color(color), material(material),
    linear_depth(linear_depth), lighting(lighting),
    depth_stencil(depth_stencil), depth_stencil_rbo(0),
    indirect_lighting(indirect_lighting)
{
    // Validate textures first
    if(
        normal != nullptr && (
        normal->get_size() != size ||
        normal->get_external_format() != GL_RG ||
        normal->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible normal");

    if(
        color != nullptr && (
        color->get_size() != size ||
        color->get_external_format() != GL_RGB ||
        color->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible color");

    if(
        material != nullptr && (
        material->get_size() != size ||
        material->get_external_format() != GL_RGBA ||
        material->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible material");

    if(lighting != nullptr && (
        lighting->get_size() != size ||
        lighting->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible lighting");

    if(linear_depth != nullptr && (
        linear_depth->get_size() != size ||
        linear_depth->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible linear_depth");

    if(
        depth_stencil != nullptr && (
        depth_stencil->get_size() != size ||
        depth_stencil->get_internal_format() != GL_DEPTH24_STENCIL8 ||
        depth_stencil->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible depth_stencil");

    if(indirect_lighting != nullptr && (
        indirect_lighting->get_size() != size ||
        indirect_lighting->get_target() != GL_TEXTURE_2D)
    ) throw std::runtime_error("Incompatible indirect_lighting");

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

    if(color)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            color->get_target(),
            color->get_texture(),
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

    if(indirect_lighting)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT5,
            indirect_lighting->get_target(),
            indirect_lighting->get_texture(),
            0
        );
    }

    set_draw(DRAW_LIGHTING);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("GBuffer is incomplete!");

    reinstate_current_fbo();
}

gbuffer::gbuffer(gbuffer&& other)
:   render_target(other), mode(other.mode),
    normal(other.normal), color(other.color), material(other.material),
    linear_depth(other.linear_depth), lighting(other.lighting),
    depth_stencil(other.depth_stencil),
    depth_stencil_rbo(other.depth_stencil_rbo),
    indirect_lighting(other.indirect_lighting),
    normal_index(other.normal_index), color_index(other.color_index),
    material_index(other.material_index),
    linear_depth_index(other.linear_depth_index),
    lighting_index(other.lighting_index),
    indirect_lighting_index(other.indirect_lighting_index)
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
texture* gbuffer::get_color() const { return color; }
texture* gbuffer::get_material() const { return material; }
texture* gbuffer::get_linear_depth() const { return linear_depth; }
texture* gbuffer::get_lighting() const { return lighting; }
texture* gbuffer::get_depth_stencil() const { return depth_stencil; }
texture* gbuffer::get_indirect_lighting() const { return indirect_lighting; }

int gbuffer::get_normal_index() const { return normal_index; }
int gbuffer::get_color_index() const { return color_index; }
int gbuffer::get_material_index() const { return material_index; }
int gbuffer::get_linear_depth_index() const { return linear_depth_index; }
int gbuffer::get_lighting_index() const { return lighting_index; }
int gbuffer::get_indirect_lighting_index() const {
    return indirect_lighting_index;
}

void gbuffer::bind_textures(const sampler& fb_sampler, unsigned& index) const
{
    if(depth_stencil) fb_sampler.bind(*depth_stencil, index++);
    if(color) fb_sampler.bind(*color, index++);
    if(normal) fb_sampler.bind(*normal, index++);
    if(material) fb_sampler.bind(*material, index++);
    if(linear_depth) fb_sampler.bind(*linear_depth, index++);
    if(lighting) fb_sampler.bind(*lighting, index++);
    if(indirect_lighting) fb_sampler.bind(*indirect_lighting, index++);
}

void gbuffer::set_uniforms(shader* s, unsigned& index) const
{
    if(depth_stencil) s->set<int>("in_depth", index++);
    if(color) s->set<int>("in_color", index++);
    if(normal) s->set<int>("in_normal", index++);
    if(material) s->set<int>("in_material", index++);
    if(linear_depth) s->set<int>("in_linear_depth", index++);
    if(lighting) s->set<int>("in_lighting", index++);
    if(indirect_lighting) s->set<int>("in_indirect_lighting", index++);
}

void gbuffer::update_definitions(shader::definition_map& def) const
{
    if(normal_index >= 0) def["NORMAL_INDEX"] = std::to_string(normal_index);
    else def.erase("NORMAL_INDEX");

    if(color_index >= 0)
        def["COLOR_INDEX"] = std::to_string(color_index);
    else def.erase("COLOR_INDEX");

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

    if(indirect_lighting_index >= 0)
        def["INDIRECT_LIGHTING_INDEX"] =
            std::to_string(indirect_lighting_index);
    else def.erase("INDIRECT_LIGHTING_INDEX");
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
    if(color_index >= 0)
        glClearBufferfv(GL_COLOR, color_index, zero);
    if(material_index >= 0)
        glClearBufferfv(GL_COLOR, material_index, zero);
    if(linear_depth_index >= 0)
        glClearBufferfv(GL_COLOR, linear_depth_index, neg_infinite);
    if(lighting_index >= 0)
        glClearBufferfv(GL_COLOR, lighting_index, zero);
    if(indirect_lighting_index >= 0)
        glClearBufferfv(GL_COLOR, indirect_lighting_index, zero);

    glClearDepth(1);
    glClearStencil(0);
    glStencilMask(0xFF);

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
    case DRAW_NONE:
        color_index = -1;
        normal_index = -1;
        material_index = -1;
        linear_depth_index = -1;
        lighting_index = -1;
        indirect_lighting_index = -1;
        break;
    case DRAW_ALL:
        if(color)
        {
            color_index = index++;
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

        if(indirect_lighting)
        {
            indirect_lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT5);
        }

        break;
    case DRAW_GEOMETRY:
        if(color)
        {
            color_index = index++;
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
        indirect_lighting_index = -1;
        break;
    case DRAW_LIGHTING:
        color_index = -1;
        normal_index = -1;
        material_index = -1;
        linear_depth_index = -1;
        if(lighting)
        {
            lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT4);
        }
        indirect_lighting_index = -1;
        break;
    case DRAW_INDIRECT_LIGHTING:
        color_index = -1;
        normal_index = -1;
        material_index = -1;
        linear_depth_index = -1;
        lighting_index = -1;
        if(indirect_lighting)
        {
            indirect_lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT5);
        }
        break;
    case DRAW_ALL_LIGHTING:
        color_index = -1;
        normal_index = -1;
        material_index = -1;
        linear_depth_index = -1;
        if(lighting)
        {
            lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT4);
        }

        if(indirect_lighting)
        {
            indirect_lighting_index = index++;
            attachments.push_back(GL_COLOR_ATTACHMENT5);
        }
        break;
    }

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
            {{"MAXIMUM", ""}, {"MINIMUM", ""}}
        );
    }

    return pool.get(
        shader::path{"fullscreen.vert", "min_max_depth.frag"},
        {{"MAXIMUM", ""}}
    );
}

void gbuffer::render_depth_mipmaps(
    shader* min_max,
    const primitive& quad,
    const sampler& fb_sampler
){
    if(!min_max || !linear_depth) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glm::uvec2 size = get_size();
    unsigned mipmap_count = calculate_mipmap_count(size);

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

} // namespace lt
