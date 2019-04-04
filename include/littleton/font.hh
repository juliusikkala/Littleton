/*
    Copyright 2019 Julius Ikkala

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
#ifndef LT_FONT_HH
#define LT_FONT_HH
#include "api.hh"
#include "context.hh"
#include "resource.hh"
#include <unordered_map>
#include <memory>
#include <boost/functional/hash.hpp>

namespace lt
{

// Mostly a container for font faces, which are the actually useful things.
class LT_API font: public resource, public glresource
{
public:
    class face: public resource
    {
    friend class font;
    public:
        face(const face& other) = delete;
        face(face&& other) = delete;
        ~face();

    protected:
        face(font* parent, unsigned ptsize, unsigned index);

        void load_impl() const override;
        void unload_impl() const override;

        void basic_load() const;
        void basic_unload() const;

        unsigned ptsize;
        unsigned index;
        font* parent;
        // FT_Face& face = static_cast<FT_Face>(ft_face);
        mutable void* ft_face;
    };

    enum render_mode
    {
        GRAYSCALE = 0,
        SUBPIXEL,
        ALIASED
    };

    font(context& ctx, const std::string& path, render_mode mode = GRAYSCALE);
    font(
        context& ctx,
        const uint8_t* data, size_t size,
        render_mode mode = GRAYSCALE
    );
    font(const font& other) = delete;
    font(font&& other);
    ~font();

    const face& operator()(unsigned ptsize, unsigned index = 0) const;
    size_t face_count() const;

    static font* create(
        context& ctx,
        const std::string& path,
        render_mode mode = GRAYSCALE
    );
    static font* create(
        context& ctx,
        const uint8_t* data,
        size_t size,
        render_mode mode = GRAYSCALE
    );

protected:
    explicit font(context& ctx, render_mode mode);
    using face_key = std::pair<unsigned, unsigned>;

    void basic_load(const std::string& path) const;
    void basic_load(const uint8_t* data, size_t size) const;
    // Using the 'data' member.
    void basic_load() const;
    void basic_unload(bool clear_data = true) const;

    render_mode mode;
    mutable uint8_t* data;
    mutable size_t data_size;
    mutable unsigned count;
    mutable void* meta_face;
    mutable std::unordered_map<
        face_key,
        std::unique_ptr<face>,
        boost::hash<face_key>
    > faces;
};

} // namespace lt

#endif
