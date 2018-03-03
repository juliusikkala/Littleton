#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"
#include "uniform.hh"
#include <map>
#include <vector>
#include <initializer_list>

class shader;

class shader: public resource, public glresource
{
public:
    using definition_map = std::map<std::string, std::string>;

    struct path
    {
        std::string vert;
        std::string frag;
        std::string geom;

        bool operator==(const path& other) const;
    };

    struct source
    {
        source() = default;
        source(
            const std::string& vert,
            const std::string& frag,
            const std::string& geom
        );
        source(const path& p);

        std::string vert;
        std::string frag;
        std::string geom;
    };

    shader(context& ctx);
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
    void set_block(const std::string& name, unsigned bind_point);

    // Checks that the type is compatible and binds the block.
    void set_block(
        const std::string& name,
        const uniform_block& block,
        unsigned bind_point
    );

protected:
    // Attempts to load the binary, but if that fails, falls back to the
    // source.
    void basic_load(
        const source& src,
        const std::string& binary = ""
    ) const;

    void populate_uniforms() const;

    void basic_unload() const;

    struct uniform_data
    {
        GLuint location;
        GLint size;
        GLenum type;
    };

    struct block_data
    {
        GLuint index;
        uniform_block_type type;
    };

    static GLuint current_program;
    mutable GLuint program;
    mutable std::unordered_map<std::string, uniform_data> uniforms;
    mutable std::unordered_map<std::string, block_data> blocks;
};

namespace boost 
{
    size_t hash_value(const shader::path& p);
}

#include "shader.tcc"

#endif
