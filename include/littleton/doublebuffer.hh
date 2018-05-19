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
#ifndef LT_DOUBLEBUFFER_HH
#define LT_DOUBLEBUFFER_HH
#include "api.hh"
#include "resource.hh"
#include "render_target.hh"
#include "texture.hh"

namespace lt
{

class LT_API doublebuffer: public glresource
{
public:
    doublebuffer(
        context& ctx,
        glm::uvec2 size,
        GLint internal_format
    );
    doublebuffer(doublebuffer&& other);
    ~doublebuffer();

    class target: public render_target
    {
    public:
        target(context& ctx, texture& tex);
        target(target&& other);
        ~target();

        void set_depth_stencil(texture* depth_stencil);
    };

    target& input(unsigned index = 0);
    const target& input(unsigned index = 0) const;

    texture& output(unsigned index = 0);
    const texture& output(unsigned index = 0) const;

    void set_depth_stencil(texture* depth_stencil);
    void set_depth_stencil(unsigned index, texture* depth_stencil);

    void swap();

private:
    unsigned cur_index;

    texture buffers[2];
    target targets[2];
};

} // namespace lt

#endif

