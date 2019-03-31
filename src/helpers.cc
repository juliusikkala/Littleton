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
#include "helpers.hh"
#include <cstdio>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace lt
{

std::string read_text_file(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "rb");

    if(!f)
    {
        throw std::runtime_error("Unable to open " + path);
    }

    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = new char[sz];
    if(fread(data, 1, sz, f) != sz)
    {
        delete [] data;
        throw std::runtime_error("Unable to read " + path);
    }
    fclose(f);
    std::string ret(data, sz);

    delete [] data;
    return ret;
}

bool read_binary_file(const std::string& path, uint8_t*& data, size_t& bytes)
{
    FILE* f = fopen(path.c_str(), "rb");

    if(!f) return false;

    fseek(f, 0, SEEK_END);
    bytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = new uint8_t[bytes];
    if(fread(data, 1, bytes, f) != bytes)
    {
        delete [] data;
        bytes = 0;
        return false;
    }
    fclose(f);

    return true;
}

bool write_binary_file(const std::string& path, const uint8_t* data, size_t bytes)
{
    FILE* f = fopen(path.c_str(), "wb");

    if(!f) return false;

    if(fwrite(data, 1, bytes, f) != bytes) return false;
    fclose(f);

    return true;
}

size_t count_lines(const std::string& str)
{
    return 1 + std::count(str.begin(), str.end(), '\n');
}

std::string add_line_numbers(const std::string& src)
{
    std::stringstream in(src);
    std::stringstream out;
    size_t line_count = count_lines(src);
    size_t number_width = 1 + (size_t)floor(log10(line_count));
    std::string line;

    unsigned line_number = 1;
    while(std::getline(in, line, '\n'))
    {
        out << "| " << std::setw(number_width) << line_number << " |"
            << line << std::endl;

        line_number++;
    }

    return out.str();
}

GLint internal_format_to_external_format(GLint internal_format)
{
    switch(internal_format)
    {
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_COMPRESSED_RED:
    case GL_COMPRESSED_RED_RGTC1:
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
        return GL_RED;
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_COMPRESSED_RG:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
        return GL_RG;
    case GL_RGB:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_COMPRESSED_RGB:
    case GL_COMPRESSED_SRGB:
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        return GL_RGB;
    case GL_BGR:
        return GL_BGR;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_COMPRESSED_RGBA:
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_SRGB_ALPHA:
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        return GL_RGBA;
    case GL_BGRA:
        return GL_BGRA;
    case GL_RED_INTEGER:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
        return GL_RED_INTEGER;
    case GL_RG_INTEGER:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
        return GL_RG_INTEGER;
    case GL_RGB_INTEGER:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
    case GL_RGB10_A2UI:
        return GL_RGB_INTEGER;
    case GL_BGR_INTEGER:
        return GL_BGR_INTEGER;
    case GL_RGBA_INTEGER:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        return GL_RGBA_INTEGER;
    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1:
    case GL_STENCIL_INDEX4:
    case GL_STENCIL_INDEX8:
    case GL_STENCIL_INDEX16:
        return GL_STENCIL_INDEX;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return GL_DEPTH_COMPONENT;
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return GL_DEPTH_STENCIL;
    default:
        throw std::runtime_error(
            "Unknown internal texture format "
            + std::to_string(internal_format)
        );
    }
}

GLint internal_format_compatible_type(GLint internal_format)
{
    if(internal_format == GL_DEPTH24_STENCIL8) return GL_UNSIGNED_INT_24_8;
    return GL_UNSIGNED_BYTE;
}

unsigned internal_format_channel_count(GLint internal_format)
{
    switch(internal_format)
    {
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_COMPRESSED_RED:
    case GL_COMPRESSED_RED_RGTC1:
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
    case GL_RED_INTEGER:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1:
    case GL_STENCIL_INDEX4:
    case GL_STENCIL_INDEX8:
    case GL_STENCIL_INDEX16:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return 1;
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_COMPRESSED_RG:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
    case GL_RG_INTEGER:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return 2;
    case GL_RGB:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_COMPRESSED_RGB:
    case GL_COMPRESSED_SRGB:
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
    case GL_BGR:
    case GL_RGB_INTEGER:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
    case GL_RGB10_A2UI:
    case GL_BGR_INTEGER:
        return 3;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_COMPRESSED_RGBA:
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_SRGB_ALPHA:
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
    case GL_BGRA:
    case GL_RGBA_INTEGER:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        return 4;
    default:
        throw std::runtime_error(
            "Unknown internal texture format "
            + std::to_string(internal_format)
        );
    }
}

unsigned gl_type_sizeof(GLenum type)
{
    switch(type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_HALF_FLOAT:
        return 2;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FIXED:
    case GL_FLOAT:
        return 4;
    case GL_DOUBLE:
        return 8;
    default:
        throw std::runtime_error(
            "Unknown OpenGL type " + std::to_string(type)
        );
    }
}

GLenum get_binding_name(GLenum target)
{
    switch(target)
    {
    case GL_TEXTURE_1D:
        return GL_TEXTURE_BINDING_1D;
    case GL_TEXTURE_2D:
        return GL_TEXTURE_BINDING_2D;
    case GL_TEXTURE_3D:
        return GL_TEXTURE_BINDING_3D;
    case GL_TEXTURE_1D_ARRAY:
        return GL_TEXTURE_BINDING_1D_ARRAY;
    case GL_TEXTURE_2D_ARRAY:
        return GL_TEXTURE_BINDING_2D_ARRAY;
    case GL_TEXTURE_RECTANGLE:
        return GL_TEXTURE_BINDING_RECTANGLE;
    case GL_TEXTURE_BUFFER:
        return GL_TEXTURE_BINDING_BUFFER;
    case GL_TEXTURE_CUBE_MAP:
        return GL_TEXTURE_BINDING_CUBE_MAP;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
    case GL_TEXTURE_2D_MULTISAMPLE:
        return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
    default:
        throw std::runtime_error(
            "Unknown texture target " + std::to_string(target)
        );
    }
}

bool gl_target_is_array(GLenum target)
{
    switch(target)
    {
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        return true;
    default:
        return false;
    }
}

const char* get_freetype_error(int err)
{
    #undef FTERRORS_H__
    #define FT_ERROR_START_LIST  switch(err) {
    #define FT_ERRORDEF(e, v, s) case e: return "FreeType: " s;
    #define FT_ERROR_END_LIST    }
    #include FT_ERRORS_H
    return "FreeType: Unknown error";
}

} // namespace lt
