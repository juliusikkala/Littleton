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
#include "generate_depth_mipmap.hh"
#include "primitive.hh"
#include "common_resources.hh"
#include "resource_pool.hh"
#include "gbuffer.hh"
#include "shader_pool.hh"
#include "sampler.hh"
#include "shader.hh"
#include <utility>

namespace
{
    using namespace lt;
    shader::definition_map get_min_max_definitions(gbuffer& buf)
    {
        texture* linear_depth = buf.get_linear_depth();
        if(linear_depth->get_external_format() == GL_RG)
            return {{"MAXIMUM", ""}, {"MINIMUM", ""}};
        return {{"MAXIMUM", ""}};
    }
}

namespace lt::method
{

generate_depth_mipmap::generate_depth_mipmap(gbuffer& buf, resource_pool& pool)
:   target_method(buf),
    buf(&buf),
    min_max_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "min_max_depth.frag"},
        get_min_max_definitions(buf)
    )),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void generate_depth_mipmap::execute()
{
    target_method::execute();

    texture* linear_depth = buf->get_linear_depth();
    if(!min_max_shader || !linear_depth) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    glm::uvec2 size = buf->get_size();
    unsigned mipmap_count = calculate_mipmap_count(size);

    GLenum target = linear_depth->get_target();
    GLuint tex = linear_depth->get_texture();

    min_max_shader->bind();
    min_max_shader->set("prev", fb_sampler.bind(tex));

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
        min_max_shader->set<bool>("handle_top_edge", size.y&1);
        min_max_shader->set<bool>("handle_right_edge", size.x&1);
        min_max_shader->set<bool>("handle_both_edges", (size.x&1)&&(size.y&1));

        size = glm::max(size/2u, glm::uvec2(1));
        glViewport(0,0,size.x,size.y);
        quad.draw();
    }

    // Restore original state
    size = buf->get_size();
    glViewport(0,0,size.x,size.y);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT3,
        target, tex, 0
    );

    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000);

    buf->set_draw(buf->get_draw());
}

std::string generate_depth_mipmap::get_name() const
{
    return "generate_depth_mipmap";
}

} // namespace lt::method

