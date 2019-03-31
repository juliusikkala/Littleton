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
#include "font.hh"
#include "helpers.hh"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace lt
{

font::face::face(void* ft_face)
: ft_face(ft_face)
{
}

font::face::face(face&& other): ft_face(other.ft_face)
{
    other.ft_face = nullptr;
}

font::face::~face()
{
    if(ft_face)
        FT_Done_Face(static_cast<FT_Face>(ft_face));
}

font::font(context& ctx, const std::string& path)
: glresource(ctx)
{
    basic_load(path);
}

font::font(context& ctx, const uint8_t* data, size_t size)
: glresource(ctx)
{
    basic_load(data, size);
}

font::font(font&& other)
: glresource(other.get_context())
{
    other.load();
    data = std::move(other.data);
    faces = std::move(other.faces);

    other.data.clear();
    other.faces.clear();
}

font::~font()
{
    basic_unload();
}

font::operator face&()
{
    load();
    return faces[0];
}

font::operator const face&()
{
    load();
    return faces[0];
}

font::face& font::operator[](unsigned i)
{
    load();
    return faces[i];
}

const font::face& font::operator[](unsigned i) const
{
    load();
    return faces[i];
}

size_t font::face_count() const
{
    load();
    return faces.size();
}

class file_font: public font
{
public:
    file_font(context& ctx, const std::string& path)
    : font(ctx), path(path)
    {}

protected:
    void load_impl() const override
    {
        basic_load(path);
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    std::string path;
};

font* font::create(context& ctx, const std::string& path)
{
    return new file_font(ctx, path);
}

class data_font: public font
{
public:
    data_font(context& ctx, const uint8_t* data, size_t size)
    : font(ctx)
    {
        this->data.resize(size);
        memcpy(this->data.data(), data, size);
    }

protected:
    void load_impl() const override
    {
        basic_load();
    }

    void unload_impl() const override
    {
        basic_unload(false);
    }
};

font* font::create(context& ctx, const uint8_t* data, size_t size)
{
    return new data_font(ctx, data, size);
}

font::font(context& ctx)
: glresource(ctx)
{
}

void font::basic_load(const std::string& path) const
{
    if(faces.size() != 0) return;

    uint8_t* data = nullptr;
    size_t size = 0;
    if(!read_binary_file(path, data, size))
        throw std::runtime_error("Failed to load font data from file " + path);

    basic_load(data, size);
    delete [] data;
}

void font::basic_load(const uint8_t* data, size_t size) const
{
    if(faces.size() != 0) return;

    this->data.resize(size);
    memcpy(this->data.data(), data, size);
    basic_load();
}

void font::basic_load() const
{
    if(faces.size() != 0) return;

    FT_Face temp;

    FT_Library ft = *static_cast<FT_Library*>(get_context().freetype());
    FT_Error err = FT_New_Memory_Face(ft, data.data(), data.size(), -1, &temp);
    if(err) throw std::runtime_error(get_freetype_error(err));

    unsigned face_count = temp->num_faces;
    FT_Done_Face(temp);

    for(unsigned i = 0; i < face_count; ++i)
    {
        FT_Face f;
        err = FT_New_Memory_Face(ft, data.data(), data.size(), i, &f);
        if(err) throw std::runtime_error(get_freetype_error(err));
        faces.push_back(face(static_cast<void*>(f)));
    }
}

void font::basic_unload(bool clear_data) const
{
    faces.clear();
    if(clear_data)
        data.clear();
}

} // namespace lt
