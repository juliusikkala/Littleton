#include "shader.hh"
#include <stdexcept>

shader_data::shader_data(
    const std::string& vert_src,
    const std::string& frag_src
): program(-1) {}

shader_data::~shader_data()
{
    unload();
}

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

void shader_data::load()
{
    if(program == -1)
    {
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

        // TODO: bind data locations
        
        glLinkProgram(program);
        throw_program_error(program);
    }
}

void shader_data::unload()
{
    if(program != -1)
    {
        glDeleteShader(program);
        program = -1;
    }
}

shader::shader(resource_manager& manager, const std::string& resource_name)
: resource<shader_data>(manager, resource_name) {}

shader::~shader() { }
