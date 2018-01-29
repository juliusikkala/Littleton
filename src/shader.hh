#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"
#include "uniform.hh"
#include <map>

class shader;

class shader: public resource
{
public:
    using definition_map = std::map<std::string, std::string>;
    shader();
    shader(
        const std::string& vert_src,
        const std::string& frag_src,
        const definition_map& definitions = {}
    );
    shader(shader&& other);
    ~shader();

    GLuint get_program() const;

    static shader* create(
        const std::string& vert_src,
        const std::string& frag_src,
        const definition_map& definitions = {}
    );

    static shader* create_from_file(
        const std::string& vert_path,
        const std::string& frag_path,
        const definition_map& definitions = {}
    );

    void bind() const;
    static void unbind();

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
    void basic_load(
        const std::string& vert_src,
        const std::string& frag_src,
        const definition_map& definitions
    ) const;

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

#include "shader.tcc"

#endif
