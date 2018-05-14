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
#ifndef LT_RENDER_TARGET_HH
#define LT_RENDER_TARGET_HH
#include "glheaders.hh"
#include "resource.hh"
#include "math.hh"

namespace lt
{

class render_target: public glresource
{
public:
    render_target(
        context& ctx,
        glm::uvec2 size = glm::uvec2(0)
    );
    virtual ~render_target();

    void bind(GLenum target = GL_FRAMEBUFFER);
    void unbind();
    bool is_bound(GLenum target = GL_FRAMEBUFFER) const;

    glm::uvec2 get_size() const;
    float get_aspect() const;

    GLuint get_fbo() const;

    static GLint get_current_read_fbo();
    static GLint get_current_write_fbo();
    static void reinstate_current_fbo();

protected:
    GLuint fbo;
    glm::uvec2 size;

    static GLint current_read_fbo;
    static GLint current_write_fbo;
};

} // namespace lt

#endif
