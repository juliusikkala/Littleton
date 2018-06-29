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
#ifndef LT_SHADER_HH
#define LT_SHADER_HH
#include "api.hh"
#include "glheaders.hh"
#include "resource.hh"
#include "uniform.hh"
#include <map>
#include <vector>
#include <initializer_list>

namespace lt
{
class gpu_buffer;
class texture;

class LT_API shader: public resource, public glresource
{
public:
    using definition_map = std::map<std::string, std::string>;

    struct LT_API path
    {
        path() = default;
        path(
            const std::string& vert,
            const std::string& frag,
            const std::string& geom = "",
            const std::string& comp = ""
        );

        explicit path(const std::string& comp);

        std::string vert;
        std::string frag;
        std::string geom;
        std::string comp;

        bool operator==(const path& other) const;
    };

    struct LT_API source
    {
        source() = default;
        source(
            const std::string& vert,
            const std::string& frag,
            const std::string& geom = "",
            const std::string& comp = ""
        );
        explicit source(const std::string& comp);
        explicit source(const path& p);

        std::string vert;
        std::string frag;
        std::string geom;
        std::string comp;
    };

    explicit shader(context& ctx);
    shader(
        context& ctx,
        const source& s,
        const definition_map& definitions = {},
        const std::vector<std::string>& include_path = {},
        const std::string& binary_path = ""
    );
    shader(
        context& ctx,
        const path& s,
        const definition_map& definitions = {},
        const std::vector<std::string>& include_path = {},
        const std::string& binary_path = ""
    );
    shader(shader&& other);
    ~shader();

    GLuint get_program() const;

    static shader* create(
        context& ctx,
        const source& s,
        const definition_map& definitions = {},
        const std::vector<std::string>& include_path = {},
        const std::string& binary_path = ""
    );

    static shader* create(
        context& ctx,
        const path& p,
        const definition_map& definitions = {},
        const std::vector<std::string>& include_path = {},
        const std::string& binary_path = ""
    );

    void bind() const;
    static void unbind();

    void write_binary(const std::string& path) const;

    template<typename T>
    bool is_compatible(const std::string& name, size_t count = 1) const;

    template<typename T>
    void set(const std::string& name, const T& value);

    template<typename T>
    void set(
        const std::string& name,
        size_t count,
        const T* value
    );

    bool block_exists(const std::string& name) const;
    uniform_block_type get_block_type(const std::string& name) const;

    // Performs no safety checks! You must manually bind the block!
    void set_uniform_block(const std::string& name, unsigned bind_point);

    // Checks that the type is compatible and binds the block.
    void set_uniform_block(
        const std::string& name,
        const uniform_block& block,
        unsigned bind_point
    );

    // Performs no safety checks! You must manually bind the block!
    void set_storage_block(const std::string& name, unsigned bind_point);

    // Binds the block at the same time. Performs no type checking.
    void set_storage_block(
        const std::string& name,
        const gpu_buffer& block,
        unsigned bind_point
    );

    // Don't use this for normal textures! This isn't related to the sampler*
    // uniforms at all! It's meant for the image* types, mostly found in compute
    // shaders.
    void set_image_texture(
        const std::string& name,
        const texture& tex,
        unsigned bind_point,
        GLenum access = GL_READ_ONLY
    );

    // Executes the compute shader
    void compute_dispatch(uvec3 work_group_size);

protected:
    // Attempts to load the binary, but if that fails, falls back to the
    // source.
    void basic_load(
        const source& src,
        const std::string& binary = ""
    ) const;

    void populate_uniforms() const;
    GLuint get_storage_block(const std::string& name) const;

    void basic_unload() const;

    struct uniform_data
    {
        GLuint location;
        GLint size;
        GLenum type;
    };

    struct uniform_block_data
    {
        GLuint index;
        uniform_block_type type;
    };

    static GLuint current_program;
    mutable GLuint program;
    mutable std::unordered_map<std::string, uniform_data> uniforms;
    mutable std::unordered_map<std::string, uniform_block_data> uniform_blocks;
    mutable std::unordered_map<std::string, GLuint /*index*/> storage_blocks;
};

} // namespace lt

namespace boost 
{
    size_t hash_value(const lt::shader::path& p);
}

#include "shader.tcc"

#endif
