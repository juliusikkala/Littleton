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
#ifndef LT_FRAMEBUFFER_HH
#define LT_FRAMEBUFFER_HH
#include "api.hh"
#include "render_target.hh"
#include <vector>
#include <map>
#include <memory>
#include <variant>

namespace lt
{

class texture;
class LT_API framebuffer: public render_target
{
public:
    struct target_specifier
    {
        target_specifier(
            GLint format = GL_RGBA,
            bool as_texture = false
        );

        target_specifier(texture* use_texture);

        GLint format;
        bool as_texture;
        texture* use_texture;

        bool operator==(const target_specifier& other) const;
    };

    using target_specification_map = std::map<GLenum, target_specifier>;

    framebuffer(
        context& ctx,
        glm::uvec2 size,
        const target_specification_map& target_specifications = {},
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D
    );

    framebuffer(
        context& ctx,
        glm::uvec3 dimensions,
        const target_specification_map& target_specifications,
        unsigned samples,
        GLenum target
    );

    framebuffer(framebuffer&& f);
    ~framebuffer();

    const target_specification_map& get_target_specifications() const;
    unsigned get_samples() const;
    GLenum get_target() const;

    texture* get_texture_target(GLenum attachment) const;

private:
    target_specification_map target_specifications;
    unsigned samples;
    GLenum target;

    std::vector<std::unique_ptr<texture>> owned_textures;

    std::map<GLenum, std::variant<texture*, GLuint>> targets;
};

} // namespace lt

namespace boost
{
    size_t hash_value(const lt::framebuffer::target_specifier& t);
}

#endif
