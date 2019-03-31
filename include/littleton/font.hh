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

namespace lt
{

class LT_API font: public resource, public glresource
{
public:
    class face
    {
    friend class font;
    public:
        face(const face& other) = delete;
        face(face&& other);
        ~face();

    protected:
        face(void* ft_face);
        // FT_Face& face = static_cast<FT_Face>(face);
        void* ft_face;
    };

    font(context& ctx, const std::string& path);
    font(context& ctx, const uint8_t* data, size_t size);
    font(const font& other) = delete;
    font(font&& other);
    ~font();

    operator face&(); 
    operator const face&(); 
    face& operator[](unsigned i); 
    const face& operator[](unsigned i) const;
    size_t face_count() const;

    static font* create(context& ctx, const std::string& path);
    static font* create(context& ctx, const uint8_t* data, size_t size);

protected:
    explicit font(context& ctx);

    void basic_load(const std::string& path) const;
    void basic_load(const uint8_t* data, size_t size) const;
    // Using the 'data' member.
    void basic_load() const;
    void basic_unload(bool clear_data = true) const;

    mutable std::vector<uint8_t> data;
    mutable std::vector<face> faces;
};

} // namespace lt

#endif
