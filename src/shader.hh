#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"
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

    template<typename T>
    static bool is_compatible(GLenum type, GLint size, size_t count = 1);

    mutable GLuint program;
    mutable std::unordered_map<
        std::string,
        uniform_data
    > uniforms;
};

#include "shader.tcc"

#endif
