#include "shader.hh"
#include "helpers.hh"
#include <stdexcept>
#include <sstream>
#include <set>
#include <regex>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

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
        throw std::runtime_error(
            name + ": " + err_str + "\n" + add_line_numbers(src)
        );
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
    const shader::definition_map& definitions
){
    std::string definition_src = generate_definition_src(definitions);

    size_t offset = source.find("#version");
    if(offset == std::string::npos) return definition_src + source;

    offset = source.find_first_of('\n', offset) + 1;

    return source.substr(0, offset) + definition_src + source.substr(offset);
}

static std::string process_source(
    const std::string& source,
    const shader::definition_map& definitions,
    const std::vector<std::string>& include_path
){
    if(source.empty()) return source;
    std::string processed = remove_comments(source);
    processed = splice_definitions(processed, definitions);
    
    std::set<std::string> included;

    static const std::regex include_regex(
        "#\\s*include\\s*(\"(.*)\"|([_a-zA-Z][_a-zA-Z0-9]*))",
        std::regex::optimize
    );

    std::smatch include_match;
    std::smatch include_define_match;
    while(true)
    {
        std::string include_file;
        
        if(std::regex_search(processed, include_match, include_regex))
        {
            if(include_match[2].length())
            {
                include_file = include_match[2];
            }
            else if(include_match[3].length())
            {
                std::string name = include_match[3];
                auto it = definitions.find(name);
                if(it == definitions.end())
                {
                    processed.erase(
                        include_match[0].first,
                        include_match[0].second
                    );
                    continue;
                }
                include_file = it->second;
            }
            else break;
        }
        else break;

        if(included.count(include_file))
        {
            processed.erase(include_match[0].first, include_match[0].second);
            continue;
        }

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


bool shader::path::operator==(const path& other) const
{
    return vert == other.vert && frag == other.frag;
}

size_t boost::hash_value(const shader::path& p)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, p.vert);
    boost::hash_combine(seed, p.frag);
    return seed;
}

shader::path::path(
    const std::string& vert,
    const std::string& frag,
    const std::string& geom
): vert(vert), frag(frag), geom(geom) {}

shader::source::source(
    const std::string& vert,
    const std::string& frag,
    const std::string& geom
): vert(vert), frag(frag), geom(geom) {}

shader::source::source(const path& p)
: vert(read_text_file(p.vert)), frag(read_text_file(p.frag)),
  geom(p.geom.empty() ? "" : read_text_file(p.geom))
{
}


GLuint shader::current_program = 0;

shader::shader(context& ctx): glresource(ctx), program(0) {}

shader::shader(
    context& ctx,
    const source& s,
    const definition_map& definitions,
    const std::vector<std::string>& include_path,
    const std::string& binary_path
): glresource(ctx), program(0)
{
    basic_load({
        process_source(s.vert, definitions, include_path),
        process_source(s.frag, definitions, include_path),
        process_source(s.geom, definitions, include_path)
    }, binary_path);
}

shader::shader(
    context& ctx,
    const path& p,
    const definition_map& definitions,
    const std::vector<std::string>& include_path,
    const std::string& binary_path
): shader(ctx, source(p), definitions, include_path, binary_path) {}

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

void shader::write_binary(const std::string& path) const
{
    load();
    GLint length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);

    if(!length) return;

    uint8_t* binary = new uint8_t[length + sizeof(GLenum)];
    uint8_t* binary_data = binary + sizeof(GLenum);
    GLenum* binary_format = (GLenum*)binary;

    glGetProgramBinary(program, length, nullptr, binary_format, binary_data);

    boost::filesystem::path p(path);
    boost::filesystem::create_directories(p.parent_path());

    write_binary_file(path, binary, length + sizeof(GLenum));

    delete [] binary;
}

class src_shader: public shader
{
public:
    src_shader(
        context& ctx,
        const source& s,
        const definition_map& definitions,
        const std::vector<std::string>& include_path,
        const std::string& binary_path
    ): shader(ctx),
       src(
           process_source(s.vert, definitions, include_path),
           process_source(s.frag, definitions, include_path),
           process_source(s.geom, definitions, include_path)
       ),
       binary_path(binary_path)
    { }

protected:
    void load_impl() const override
    {
        basic_load(src, binary_path);
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    source src;
    std::string binary_path;
};

shader* shader::create(
    context& ctx,
    const source& s,
    const definition_map& definitions,
    const std::vector<std::string>& include_path,
    const std::string& binary_path
){
    return new src_shader(ctx, s, definitions, include_path, binary_path);
}

shader* shader::create(
    context& ctx,
    const path& p,
    const definition_map& definitions,
    const std::vector<std::string>& include_path,
    const std::string& binary_path
){
    std::vector<std::string> extended_include_path = {
        boost::filesystem::path(p.vert).parent_path().string(),
        boost::filesystem::path(p.frag).parent_path().string(),
        boost::filesystem::path(p.geom).parent_path().string()
    };
    extended_include_path.insert(
        extended_include_path.end(),
        include_path.begin(),
        include_path.end()
    );
    return new src_shader(
        ctx,
        source(p),
        definitions,
        extended_include_path,
        binary_path
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

static void remove_index_brackets(std::string& name)
{
    // Remove [0]
    if(name.length() > 3 && name.compare(name.length()-3, 3, "[0]") == 0)
    {
        name = name.substr(0, name.length()-3);
    }
}

void shader::basic_load(
    const source& src,
    const std::string& binary_path
) const
{
    if(program) return;

    program = glCreateProgram();

    bool load_from_source = true;

    // Attempt to load the binary
    if(!binary_path.empty())
    {
        uint8_t* binary;
        size_t length;

        if(read_binary_file(binary_path, binary, length))
        {
            uint8_t* binary_data = binary + sizeof(GLenum);
            GLenum binary_format = *(GLenum*)binary;
            length -= sizeof(GLenum);

            glProgramBinary(
                program,
                binary_format,
                binary_data,
                length
            );

            delete [] binary;

            if(glGetError() == GL_NO_ERROR)
            {
                GLint status = GL_FALSE;
                glGetProgramiv(program, GL_LINK_STATUS, &status);
                if(status == GL_TRUE)
                {
                    load_from_source = false;
                }
            }

            if(load_from_source) std::remove(binary_path.c_str());
        }
    }

    if(load_from_source)
    {
        const char* vsrc = src.vert.c_str();
        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vsrc, NULL);
        glCompileShader(vshader);
        throw_shader_error(vshader, "Vertex shader", src.vert);
        glAttachShader(program, vshader);
        glDeleteShader(vshader);

        const char* fsrc = src.frag.c_str();
        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fsrc, NULL);
        glCompileShader(fshader);
        throw_shader_error(fshader, "Fragment shader", src.frag);
        glAttachShader(program, fshader);
        glDeleteShader(fshader);
        
        if(!src.geom.empty())
        {
            const char* gsrc = src.geom.c_str();
            GLuint gshader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gshader, 1, &gsrc, NULL);
            glCompileShader(gshader);
            throw_shader_error(gshader, "Geometry shader", src.geom);
            glAttachShader(program, gshader);
            glDeleteShader(gshader);
        }

        glLinkProgram(program);
        throw_program_error(program, "Shader program");

        if(!binary_path.empty())
        {
            write_binary(binary_path);
        }
    }

    populate_uniforms();
}

void shader::populate_uniforms() const
{
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
        std::string shortened_name(name);

        remove_index_brackets(shortened_name);

        struct uniform_data data;
        data.location = glGetUniformLocation(program, name);
        delete [] name;

        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_SIZE, (GLint*)&data.size
        );

        glGetActiveUniformsiv(
            program, 1, &i, GL_UNIFORM_TYPE, (GLint*)&data.type
        );
        uniforms[shortened_name] = data;
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
            remove_index_brackets(shortened_name);

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
