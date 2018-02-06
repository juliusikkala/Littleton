#include "shader.hh"
#include "helpers.hh"
#include <stdexcept>
#include <sstream>
#include <set>
#include <regex>
#include <boost/filesystem.hpp>

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

static std::string generate_definition_src(
    const shader::definition_map& definitions
){
    std::stringstream ss;
    for(auto& pair: definitions)
        ss << "#define " << pair.first << " " << pair.second << std::endl;
    return ss.str();
}

static std::string remove_comments(const std::string& source)
{
    std::string processed = source;
    while(1)
    {
        size_t block = processed.find("/*");
        size_t line = processed.find("//");

        if(block == std::string::npos && line == std::string::npos)
            break;

        if(block < line)
        {
            size_t block_end = processed.find("*/", block+2);
            processed.erase(block, block_end-block+2);
        }
        else
        {
            size_t line_end = processed.find("\n", line+2);
            processed.erase(line, line_end-line);
        }
    }
    return processed;
}

static std::string splice_definitions(
    const std::string& source,
    const std::string& definitions
){
    size_t offset = source.find("#version");
    if(offset == std::string::npos) return definitions + source;

    offset = source.find_first_of('\n', offset) + 1;

    return source.substr(0, offset) + definitions + source.substr(offset);
}

static std::string process_source(
    const std::string& source,
    const std::string& definitions,
    const std::vector<std::string>& include_path
){
    std::string processed = remove_comments(source);
    processed = splice_definitions(processed, definitions);
    
    std::set<std::string> included;

    static const std::regex include_regex(
        "#\\s*include\\s*\"(.*)\"",
        std::regex::optimize
    );

    std::smatch include_match;
    while(std::regex_search(processed, include_match, include_regex))
    {
        std::string include_file = include_match[1];
        if(included.count(include_file)) continue;

        std::string include_src;
        bool success = false;

        for(const std::string& path: include_path)
        {
            try
            {
                boost::filesystem::path dir(path);
                boost::filesystem::path file(include_file);

                include_src = read_text_file((dir/file).string());
            }
            catch(...)
            {
                continue;
            }

            success = true;
            break;
        }
        if(!success)
            throw std::runtime_error(
                "Unable to find file " + include_file + " for #include"
            );

        included.insert(include_file);

        processed.replace(
            include_match[0].first,
            include_match[0].second,
            remove_comments(include_src)
        );
    }

    return processed;
}

GLuint shader::current_program = 0;

shader::shader(context& ctx): glresource(ctx), program(0) {}

shader::shader(
    context& ctx,
    const std::string& vert_src,
    const std::string& frag_src,
    const definition_map& definitions,
    const std::vector<std::string>& include_path
): glresource(ctx), program(0)
{
    std::string definition_src = generate_definition_src(definitions);
    basic_load(
        process_source(vert_src, definition_src, include_path),
        process_source(frag_src, definition_src, include_path)
    );
}

shader::shader(shader&& other)
: glresource(other.get_context())
{
    other.load();
    program = other.program;
    uniforms = std::move(other.uniforms);
    blocks = std::move(other.blocks);
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
        context& ctx,
        const std::string& vert_src,
        const std::string& frag_src,
        const std::string& definition_src,
        const std::vector<std::string>& include_path
    ): shader(ctx),
       vert_src(process_source(vert_src, definition_src, include_path)),
       frag_src(process_source(frag_src, definition_src, include_path))
    { }

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
    context& ctx,
    const std::string& vert_src,
    const std::string& frag_src,
    const definition_map& definitions,
    const std::vector<std::string>& include_path
){
    std::string definition_src = generate_definition_src(definitions);
    return new src_shader(
        ctx,
        vert_src,
        frag_src,
        definition_src,
        include_path
    );
}

shader* shader::create_from_file(
    context& ctx,
    const std::string& vert_path,
    const std::string& frag_path,
    const definition_map& definitions,
    const std::vector<std::string>& include_path
){
    std::string definition_src = generate_definition_src(definitions);
    std::vector<std::string> extended_include_path = {
        boost::filesystem::path(vert_path).parent_path().string(),
        boost::filesystem::path(frag_path).parent_path().string()
    };
    extended_include_path.insert(
        extended_include_path.end(),
        include_path.begin(),
        include_path.end()
    );
    return new src_shader(
        ctx,
        read_text_file(vert_path),
        read_text_file(frag_path),
        definition_src,
        extended_include_path
    );
}

bool shader::block_exists(const std::string& name) const
{
    load();
    return blocks.find(name) != blocks.end();
}

uniform_block_type shader::get_block_type(const std::string& name) const
{
    load();
    auto it = blocks.find(name);
    if(it == blocks.end())
        throw std::runtime_error("No such block type \"" + name +"\"");

    return it->second.type;
}

void shader::set_block(const std::string& name, unsigned bind_point)
{
    load();
    auto it = blocks.find(name);
    if(it == blocks.end()) return;

    bind();
    glUniformBlockBinding(program, it->second.index, bind_point);
}

void shader::set_block(
    const std::string& name,
    const uniform_block& block,
    unsigned bind_point
){
    load();
    auto it = blocks.find(name);
    if(it == blocks.end()) return;
    if(it->second.type == block.get_type())
        throw std::runtime_error("Incorrect block type for " + name);

    bind();
    block.bind(bind_point);
    glUniformBlockBinding(program, it->second.index, bind_point);
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

    throw_shader_error(vshader, "Vertex shader", vert_src);
    throw_shader_error(fshader, "Fragment shader", frag_src);

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

    // Read uniforms in default block (they can be set separately by the user)
    for(GLuint i = 0; i < uniform_count; ++i)
    {
        GLint length = 0;
        GLint block = 0;
        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_BLOCK_INDEX, (GLint*)&block
        );
        // Not in default block
        if(block != -1) continue;

        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_NAME_LENGTH, (GLint*)&length
        );
        char* name = new char[length];
        glGetActiveUniformName(program, i, length, nullptr, name);

        struct uniform_data data;
        data.location = glGetUniformLocation(program, name);

        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_SIZE, (GLint*)&data.size
        );

        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_TYPE, (GLint*)&data.type
        );

        uniforms[name] = data;

        delete [] name;
    }

    // Read uniform blocks
    GLuint block_count = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, (GLint*)&block_count);

    std::vector<GLint> indices;

    for(GLuint i = 0; i < block_count; ++i)
    {
        GLint length = 0;
        glGetActiveUniformBlockiv(
            program, i, GL_UNIFORM_BLOCK_NAME_LENGTH, (GLint*)&length
        );
        char* name = new char[length];
        glGetActiveUniformBlockName(program, i, length, nullptr, name);
        std::string name_prefix = std::string(name) + ".";

        std::unordered_map<std::string, uniform_block_type::uniform_info> info;
        GLint size = 0;
        glGetActiveUniformBlockiv(
            program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size
        );

        GLint count = 0;
        glGetActiveUniformBlockiv(
            program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &count
        );
        indices.resize(count);
        glGetActiveUniformBlockiv(
            program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data()
        );

        // Add all the uniforms in this block to 'info'
        for(GLuint ui: indices)
        {
            glGetActiveUniformsiv(
                program, 1, &ui, GL_UNIFORM_NAME_LENGTH, (GLint*)&length
            );
            char* name = new char[length];
            glGetActiveUniformName(program, ui, length, nullptr, name);
            std::string shortened_name(name);
            delete [] name;

            // Remove prefix
            if(shortened_name.compare(
                0, name_prefix.length(), name_prefix
            ) == 0)
            {
                shortened_name = shortened_name.substr(name_prefix.length());
            }
            // Remove [0]
            if(
                shortened_name.length() > 3 &&
                shortened_name.compare(shortened_name.length()-3, 3, "[0]") == 0
            ){
                shortened_name =
                    shortened_name.substr(0, shortened_name.length()-3);
            }

            uniform_block_type::uniform_info data;

            glGetActiveUniformsiv(
                program, 1, &ui, GL_UNIFORM_OFFSET, &data.offset
            );

            glGetActiveUniformsiv(program, 1, &ui, GL_UNIFORM_SIZE, &data.size);

            glGetActiveUniformsiv(
                program, 1, &ui, GL_UNIFORM_ARRAY_STRIDE, &data.array_stride
            );

            glGetActiveUniformsiv(
                program, 1, &ui, GL_UNIFORM_MATRIX_STRIDE, &data.matrix_stride
            );

            glGetActiveUniformsiv(
                program, 1, &ui, GL_UNIFORM_TYPE, (GLint*)&data.type
            );

            info[shortened_name] = data;
        }

        blocks.emplace(
            name,
            block_data{i, uniform_block_type(std::move(info), size)}
        );

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
        blocks.clear();
        glDeleteProgram(program);
        program = 0;
    }
}
