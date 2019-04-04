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

font::face::face(font* parent, unsigned ptsize, unsigned index)
: ptsize(ptsize), index(index), parent(parent), ft_face(nullptr)
{
}

font::face::~face()
{
    basic_unload();
}

void font::face::load_impl() const { basic_load(); }
void font::face::unload_impl() const { basic_unload(); }

void font::face::basic_load() const
{
    parent->load();
    if(ft_face) return;

    FT_Face f;
    FT_Library ft = *static_cast<FT_Library*>(parent->get_context().freetype());
    FT_Error err = FT_New_Memory_Face(
        ft, parent->data, parent->data_size, index, &f
    );
    if(err) throw std::runtime_error(get_freetype_error(err));

    err = FT_Set_Pixel_Sizes(f, 0, ptsize);
    if(err) throw std::runtime_error(get_freetype_error(err));

    ft_face = f;
}

void font::face::basic_unload() const
{
    if(ft_face)
    {
        FT_Done_Face(static_cast<FT_Face>(ft_face));
        ft_face = nullptr;
    }
}

font::font(context& ctx, const std::string& path, render_mode mode)
:   glresource(ctx), mode(mode), data(nullptr), data_size(0), count(0),
    meta_face(nullptr)
{
    basic_load(path);
}

font::font(context& ctx, const uint8_t* data, size_t size, render_mode mode)
:   glresource(ctx), mode(mode), data(nullptr), data_size(0), count(0),
    meta_face(nullptr)
{
    basic_load(data, size);
}

font::font(font&& other)
: glresource(other.get_context()), mode(other.mode)
{
    other.load();
    data = other.data;
    data_size = other.data_size;
    count = other.count;
    meta_face = other.meta_face;
    faces = std::move(other.faces);

    for(const auto& pair: faces)
        pair.second->parent = this;

    other.data = nullptr;
    other.data_size = 0;
    other.count = 0;
    other.meta_face = nullptr;
    other.faces.clear();
}

font::~font()
{
    basic_unload();
}

const font::face& font::operator()(unsigned ptsize, unsigned index) const
{
    load();
    face_key key(ptsize, index);
    auto it = faces.find(key);
    if(it == faces.end())
    {
        if(index >= count)
            throw std::runtime_error(
                "This font has " + std::to_string(count) + " faces, but index "
                + std::to_string(index) + " was requested."
            );
        return *faces.emplace(
            key,
            new face(const_cast<font*>(this), ptsize, index)
        ).first->second;
    }
    return *it->second;
}

size_t font::face_count() const
{
    load();
    return count;
}

class file_font: public font
{
public:
    file_font(context& ctx, const std::string& path, render_mode mode)
    : font(ctx, mode), path(path)
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

font* font::create(context& ctx, const std::string& path, render_mode mode)
{
    return new file_font(ctx, path, mode);
}

class data_font: public font
{
public:
    data_font(
        context& ctx,
        const uint8_t* data, size_t size,
        render_mode mode
    ): font(ctx, mode)
    {
        this->data = new uint8_t[size];
        this->data_size = size;
        memcpy(this->data, data, size);
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

font* font::create(
    context& ctx,
    const uint8_t* data, size_t size,
    render_mode mode
){
    return new data_font(ctx, data, size, mode);
}

font::font(context& ctx, render_mode mode)
: glresource(ctx), mode(mode), data(nullptr), data_size(0), count(0)
{
}

void font::basic_load(const std::string& path) const
{
    if(count) return;

    if(!read_binary_file(path, data, data_size))
        throw std::runtime_error("Failed to load font data from file " + path);

    basic_load();
}

void font::basic_load(const uint8_t* data, size_t size) const
{
    if(count) return;

    this->data = new uint8_t[size];
    this->data_size = size;
    memcpy(this->data, data, size);
    basic_load();
}

void font::basic_load() const
{
    if(count) return;

    FT_Face meta = nullptr;
    FT_Library ft = *static_cast<FT_Library*>(get_context().freetype());
    FT_Error err = FT_New_Memory_Face(ft, data, data_size, -1, &meta);
    if(err) throw std::runtime_error(get_freetype_error(err));

    count = meta->num_faces;
    meta_face = meta;
}

void font::basic_unload(bool clear_data) const
{
    if(meta_face)
    {
        FT_Face meta = static_cast<FT_Face>(meta_face);
        FT_Done_Face(meta);
        meta_face = nullptr;
    }

    for(const auto& pair: faces)
        pair.second->unload();

    if(clear_data)
    {
        if(data)
        {
            delete [] data;
            data = nullptr;
        }
        data_size = 0;
        count = 0;
    }
}

} // namespace lt
