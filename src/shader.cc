#include "shader.hh"
#include <stdexcept>

static void throw_shader_error(GLuint shader)
{
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(!status)
    {
        GLsizei length = 0;
        glGetShaderInfoLog(shader, 0, &length, NULL);
        char* err = new char[length+1];
        glGetShaderInfoLog(shader, length+1, &length, err);
        std::string err_str(err);
        delete [] err;
        throw std::runtime_error(err_str);
    }
}

static void throw_program_error(GLuint program)
{
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if(!status)
    {
        GLsizei length = 0;
        glGetProgramInfoLog(program, 0, &length, NULL);
        char* err = new char[length+1];
        glGetProgramInfoLog(program, length+1, &length, err);
        std::string err_str(err);
        delete [] err;
        throw std::runtime_error(err_str);
    }
}

shader::shader(): program(0) {}

shader::shader(
    const std::string& vert_src,
    const std::string& frag_src
): program(0)
{
    basic_load(vert_src, frag_src);
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

class src_shader: public shader
{
public:
    src_shader(
        const std::string& vert_src,
        const std::string& frag_src
    ): vert_src(vert_src), frag_src(frag_src) {}

    void load() const override
    {
        basic_load(vert_src, frag_src);
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    std::string vert_src;
    std::string frag_src;
};

shader* shader::create(
    const std::string& vert_src,
    const std::string& frag_src
){
    return new src_shader(vert_src, frag_src);
}

void shader::basic_load(
    const std::string& vert_src,
    const std::string& frag_src
) const {
    if(program) return;

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* vsrc = vert_src.c_str();
    const char* fsrc = frag_src.c_str();
    glShaderSource(vshader, 1, &vsrc, NULL);
    glShaderSource(fshader, 1, &fsrc, NULL);

    glCompileShader(vshader);
    glCompileShader(fshader);

    throw_shader_error(vshader);
    throw_shader_error(fshader);

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
    throw_program_error(program);
}

void shader::basic_unload() const
{
    if(program != 0)
    {
        glDeleteProgram(program);
        program = 0;
    }
}
