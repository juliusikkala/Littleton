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
#ifndef LT_HELPERS_HH
#define LT_HELPERS_HH
#include <string>
#include <vector>
#include <functional>
#include <boost/functional/hash.hpp>
#include "glheaders.hh"

namespace lt
{

std::string read_text_file(const std::string& path);
bool read_binary_file(const std::string& path, uint8_t*& data, size_t& bytes);
bool write_binary_file(const std::string& path, const uint8_t* data, size_t bytes);

template<typename T, typename Hash = boost::hash<T>>
std::string append_hash_to_path(
    const std::string& prefix,
    const T& hashable,
    const std::string& suffix = ""
);

size_t count_lines(const std::string& str);

std::string add_line_numbers(const std::string& src);

GLint internal_format_to_external_format(GLint internal_format);
GLint internal_format_compatible_type(GLint internal_format);
unsigned internal_format_channel_count(GLint internal_format);
unsigned gl_type_sizeof(GLenum type);
GLenum get_binding_name(GLenum target);
bool gl_target_is_array(GLenum target);

template<typename T>
void sorted_insert(
    std::vector<T>& vec,
    const T& value
);

template<typename T>
bool sorted_erase(
    std::vector<T>& vec,
    const T& value
);

} // namespace lt

#include "helpers.tcc"

#endif
