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
#ifndef LT_METHOD_BLIT_FRAMEBUFFER_HH
#define LT_METHOD_BLIT_FRAMEBUFFER_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../render_target.hh"

namespace lt::method
{

class LT_API blit_framebuffer: public target_method
{
public:
    enum blit_type
    {
        COLOR_ONLY = GL_COLOR_BUFFER_BIT,
        DEPTH_ONLY = GL_DEPTH_BUFFER_BIT,
        STENCIL_ONLY = GL_STENCIL_BUFFER_BIT,
        DEPTH_STENCIL = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        COLOR_DEPTH = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        COLOR_STENCIL = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        ALL = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
              GL_STENCIL_BUFFER_BIT
    };

    blit_framebuffer(
        render_target& dst,
        render_target& src,
        blit_type type = ALL
    );

    void set_blit_type(blit_type type);
    void set_src(render_target& src);

    void execute() override;

    std::string get_name() const override;

private:
    render_target* src;
    blit_type type;
};

} // namespace lt::method

#endif
