#include "shader.hh"
#include "helpers.hh"
#include <stdexcept>
#include <sstream>

static void throw_shader_error(
    GLuint shader,
    const std::string& name,
    const std::string& src
){
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        GLsizei length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* err = new char[length+1];
        glGetShaderInfoLog(shader, length+1, &length, err);
        std::string err_str(err);
        delete [] err;
        throw std::runtime_error(name + ": " + err_str + "\n" + src);
    }
}

static void throw_program_error(
    GLuint program,
    const std::string& name
){
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(status != GL_TRUE)
    {
        GLsizei length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* err = new char[length+1];
        glGetProgramInfoLog(program, length+1, &length, err);
        std::string err_str(err);
        delete [] err;
        throw std::runtime_error(name + ": " + err_str);
    }
}

GLuint shader::current_program = 0;

shader::shader(): program(0) {}

shader::shader(
    const std::string& vert_src,
    const std::string& frag_src,
    const definition_map& definitions
): program(0)
{
    basic_load(vert_src, frag_src, definitions);
}

shader::shader(shader&& other)
{
    other.load();
    program = other.program;
    uniforms = std::move(other.uniforms);
    other.program = 0;
}

shader::~shader()
{
    basic_unload();
}

GLuint shader::get_program() const
{
    load();
    return program;
}

void shader::bind() const
{
    load();
    if(program != current_program)
    {
        glUseProgram(program);
        current_program = program;
    }
}

void shader::unbind()
{
    glUseProgram(0);
    current_program = 0;
}


class src_shader: public shader
{
public:
    src_shader(
        const std::string& vert_src,
        const std::string& frag_src,
        const definition_map& definitions
    ): vert_src(vert_src), frag_src(frag_src), definitions(definitions) {}

    void load() const override
    {
        basic_load(vert_src, frag_src, definitions);
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    std::string vert_src;
    std::string frag_src;
    definition_map definitions;
};

shader* shader::create(
    const std::string& vert_src,
    const std::string& frag_src,
    const definition_map& definitions
){
    return new src_shader(vert_src, frag_src, definitions);
}

class file_shader: public shader
{
public:
    file_shader(
        const std::string& vert_path,
        const std::string& frag_path,
        const definition_map& definitions
    ): vert_path(vert_path), frag_path(frag_path), definitions(definitions) {}

    void load() const override
    {
        if(program) return;
        basic_load(
            read_text_file(vert_path),
            read_text_file(frag_path),
            definitions
        );
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    std::string vert_path;
    std::string frag_path;
    definition_map definitions;
};

shader* shader::create_from_file(
    const std::string& vert_path,
    const std::string& frag_path,
    const definition_map& definitions
){
    return new file_shader(vert_path, frag_path, definitions);
}

static std::string generate_definition_src(
    const shader::definition_map& definitions
){
    std::stringstream ss;
    for(auto& pair: definitions)
        ss << "#define " << pair.first << " " << pair.second << std::endl;
    return ss.str();
}

static std::string splice_definitions(
    const std::string& definitions,
    const std::string& source
){
    size_t offset = source.find("#version");
    if(offset == std::string::npos) return definitions + source;

    offset = source.find_first_of('\n', offset) + 1;

    return source.substr(0, offset) + definitions + source.substr(offset);
}

void shader::basic_load(
    const std::string& vert_src,
    const std::string& frag_src,
    const definition_map& definitions
) const {
    if(program) return;

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string definition_src = generate_definition_src(definitions);
    std::string vert_src_spliced =
        splice_definitions(definition_src, vert_src).c_str();
    std::string frag_src_spliced =
        splice_definitions(definition_src, frag_src).c_str();
    const char* vsrc = vert_src_spliced.c_str();
    const char* fsrc = frag_src_spliced.c_str();
    glShaderSource(vshader, 1, &vsrc, NULL);
    glShaderSource(fshader, 1, &fsrc, NULL);

    glCompileShader(vshader);
    glCompileShader(fshader);

    throw_shader_error(vshader, "Vertex shader", vert_src_spliced);
    throw_shader_error(fshader, "Fragment shader", frag_src_spliced);

    program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");
    glBindAttribLocation(program, 2, "tangent");
    glBindAttribLocation(program, 3, "uv");

    glLinkProgram(program);
    throw_program_error(program, "Shader program");

    // Populate uniforms
    GLuint uniform_count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, (GLint*)&uniform_count);

    for(GLuint i = 0; i < uniform_count; ++i)
    {
        GLint length = 0;
        glGetActiveUniformsiv(
            program,
            1,
            &i,
            GL_UNIFORM_NAME_LENGTH,
            (GLint*)&length
        );
        char* name = new char[length];
        glGetActiveUniformName(program, i, length, nullptr, name);

        struct uniform_data data;
        data.location = glGetUniformLocation(program, name);

        glGetActiveUniformsiv(
            program,
            1,
            &i,
            GL_UNIFORM_SIZE,
            (GLint*)&data.size
        );

        glGetActiveUniformsiv(
            program,
            1,
            &i,
            GL_UNIFORM_TYPE,
            (GLint*)&data.type
        );

        uniforms[name] = data;

        delete [] name;
    }
}

void shader::basic_unload() const
{
    if(program != 0)
    {
        if(program == current_program)
        {
            unbind();
        }

        uniforms.clear();
        glDeleteProgram(program);
        program = 0;
    }
}

template<>
void uniform_set_value<float>(
    GLint location, size_t count, const float* value
){
    glUniform1fv(location, count, value);
}

template<>
void uniform_set_value<glm::vec2>(
    GLint location, size_t count, const glm::vec2* value
){
    glUniform2fv(location, count, (float*)value);
}

template<>
void uniform_set_value<glm::vec3>(
    GLint location, size_t count, const glm::vec3* value
){
    glUniform3fv(location, count, (float*)value);
}

template<>
void uniform_set_value<glm::vec4>(
    GLint location, size_t count, const glm::vec4* value
){
    glUniform4fv(location, count, (float*)value);
}

template<>
void uniform_set_value<int>(
    GLint location, size_t count, const int* value
){
    glUniform1iv(location, count, value);
}

template<>
void uniform_set_value<glm::ivec2>(
    GLint location, size_t count, const glm::ivec2* value
){
    glUniform2iv(location, count, (int*)value);
}

template<>
void uniform_set_value<glm::ivec3>(
    GLint location, size_t count, const glm::ivec3* value
){
    glUniform3iv(location, count, (int*)value);
}

template<>
void uniform_set_value<glm::ivec4>(
    GLint location, size_t count, const glm::ivec4* value
){
    glUniform4iv(location, count, (int*)value);
}

template<>
void uniform_set_value<unsigned>(
    GLint location, size_t count, const unsigned* value
){
    glUniform1uiv(location, count, value);
}

template<>
void uniform_set_value<glm::uvec2>(
    GLint location, size_t count, const glm::uvec2* value
){
    glUniform2uiv(location, count, (unsigned*)value);
}

template<>
void uniform_set_value<glm::uvec3>(
    GLint location, size_t count, const glm::uvec3* value
){
    glUniform3uiv(location, count, (unsigned*)value);
}

template<>
void uniform_set_value<glm::uvec4>(
    GLint location, size_t count, const glm::uvec4* value
){
    glUniform4uiv(location, count, (unsigned*)value);
}

// Bool vectors should be set using glm::ivec instead of this.
template<>
void uniform_set_value<bool>(
    GLint location, size_t count, const bool* value
){
    int v = *value;
    glUniform1iv(location, 1, &v);
}

template<>
void uniform_set_value<glm::mat2>(
    GLint location, size_t count, const glm::mat2* value
){
    glUniformMatrix2fv(location, count, GL_FALSE, (float*)value);
}

template<>
void uniform_set_value<glm::mat3>(
    GLint location, size_t count, const glm::mat3* value
){
    glUniformMatrix3fv(location, count, GL_FALSE, (float*)value);
}

template<>
void uniform_set_value<glm::mat4>(
    GLint location, size_t count, const glm::mat4* value
){
    glUniformMatrix4fv(location, count, GL_FALSE, (float*)value);
}
