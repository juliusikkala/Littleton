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
