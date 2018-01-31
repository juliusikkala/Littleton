#include "context.hh"
#include <cstring>
#include <unordered_map>

static const std::unordered_map<GLenum, unsigned> cacheable_params({
    // Context related
    {GL_CONTEXT_FLAGS, 1},
    {GL_MAJOR_VERSION, 1},
    {GL_MAX_SERVER_WAIT_TIMEOUT, 1},
    {GL_MINOR_VERSION, 1},
    {GL_MIN_MAP_BUFFER_ALIGNMENT, 1},
    {GL_NUM_EXTENSIONS, 1},
    {GL_NUM_SHADING_LANGUAGE_VERSIONS, 1},

    // Buffer binding related
    {GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, 1},
    {GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, 1},
    {GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, 1},
    {GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, 1},
    {GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, 1},
    {GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, 1},
    {GL_MAX_UNIFORM_BUFFER_BINDINGS, 1},

    // Debug output state
    {GL_MAX_DEBUG_LOGGED_MESSAGES, 1},
    {GL_MAX_DEBUG_MESSAGE_LENGTH, 1},
    {GL_MAX_DEBUG_GROUP_STACK_DEPTH, 1},
    {GL_MAX_LABEL_LENGTH, 1},

    // Framebuffers
    {GL_DOUBLEBUFFER, 1},
    {GL_MAX_COLOR_ATTACHMENTS, 1},
    {GL_MAX_COLOR_TEXTURE_SAMPLES, 1},
    {GL_MAX_DEPTH_TEXTURE_SAMPLES, 1},
    {GL_MAX_DRAW_BUFFERS, 1},
    {GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, 1},
    {GL_MAX_FRAMEBUFFER_HEIGHT, 1},
    {GL_MAX_FRAMEBUFFER_LAYERS, 1},
    {GL_MAX_FRAMEBUFFER_SAMPLES, 1},
    {GL_MAX_FRAMEBUFFER_WIDTH, 1},
    {GL_MAX_INTEGER_SAMPLES, 1},
    {GL_MAX_SAMPLES, 1},

    // Multisampling
    {GL_MAX_SAMPLE_MASK_WORDS, 1},

    // Pixel Transfer operations
    {GL_IMPLEMENTATION_COLOR_READ_FORMAT, 1},
    {GL_IMPLEMENTATION_COLOR_READ_TYPE, 1},

    // Programs
    {GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, 1},
    {GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, 1},
    {GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_IMAGE_SAMPLES, 1},
    {GL_MAX_IMAGE_UNITS, 1},
    {GL_MAX_PROGRAM_TEXEL_OFFSET, 1},
    {GL_MAX_SHADER_STORAGE_BLOCK_SIZE, 1},
    {GL_MAX_SUBROUTINES, 1},
    {GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, 1},
    {GL_MAX_UNIFORM_BLOCK_SIZE, 1},
    {GL_MAX_UNIFORM_LOCATIONS, 1},
    {GL_MAX_VARYING_VECTORS, 1},
    {GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, 1},
    {GL_MAX_VERTEX_ATTRIB_BINDINGS, 1},
    {GL_MAX_VERTEX_ATTRIB_STRIDE, 1},
    {GL_MIN_PROGRAM_TEXEL_OFFSET, 1},
    {GL_NUM_PROGRAM_BINARY_FORMATS, 1},
    {GL_NUM_SHADER_BINARY_FORMATS, 1},
    {GL_PROGRAM_PIPELINE_BINDING, 1},
    {GL_SHADER_COMPILER, 1},
    {GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, 1},
    {GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 1},

    // Shaders
    {GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, 1},
    {GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, 1},

    {GL_MAX_COMPUTE_ATOMIC_COUNTERS, 1},
    {GL_MAX_FRAGMENT_ATOMIC_COUNTERS, 1},
    {GL_MAX_GEOMETRY_ATOMIC_COUNTERS, 1},
    {GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, 1},
    {GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, 1},
    {GL_MAX_VERTEX_ATOMIC_COUNTERS, 1},
    {GL_MAX_COMBINED_ATOMIC_COUNTERS, 1},

    {GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, 1},
    {GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, 1},
    {GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, 1},
    {GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS, 1},
    {GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS, 1},
    {GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, 1},

    {GL_MAX_COMPUTE_IMAGE_UNIFORMS, 1},
    {GL_MAX_FRAGMENT_IMAGE_UNIFORMS, 1},
    {GL_MAX_GEOMETRY_IMAGE_UNIFORMS, 1},
    {GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, 1},
    {GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, 1},
    {GL_MAX_VERTEX_IMAGE_UNIFORMS, 1},
    {GL_MAX_COMBINED_IMAGE_UNIFORMS, 1},

    {GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, 1},
    {GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, 1},

    {GL_MAX_COMPUTE_UNIFORM_COMPONENTS, 1},
    {GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 1},
    {GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, 1},
    {GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, 1},
    {GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, 1},
    {GL_MAX_VERTEX_UNIFORM_COMPONENTS, 1},

    {GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 1},
    {GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 1},

    {GL_MAX_COMPUTE_UNIFORM_BLOCKS, 1},
    {GL_MAX_FRAGMENT_UNIFORM_BLOCKS, 1},
    {GL_MAX_GEOMETRY_UNIFORM_BLOCKS, 1},
    {GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, 1},
    {GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, 1},
    {GL_MAX_VERTEX_UNIFORM_BLOCKS, 1},
    {GL_MAX_COMBINED_UNIFORM_BLOCKS, 1},

    // Compute shaders
    {GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, 1},
    {GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1},
    {GL_MAX_COMPUTE_WORK_GROUP_COUNT, 3},
    {GL_MAX_COMPUTE_WORK_GROUP_SIZE, 3},

    // Fragment shaders
    {GL_MAX_FRAGMENT_INPUT_COMPONENTS, 1},
    {GL_MAX_FRAGMENT_UNIFORM_VECTORS, 1},
    {GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET, 1},
    {GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET, 1},

    // Geometry shaders
    {GL_MAX_GEOMETRY_INPUT_COMPONENTS, 1},
    {GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, 1},
    {GL_MAX_GEOMETRY_OUTPUT_VERTICES, 1},
    {GL_MAX_GEOMETRY_SHADER_INVOCATIONS, 1},
    {GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, 1},
    {GL_MAX_VERTEX_STREAMS, 1},

    // Tessellation control shaders
    {GL_MAX_PATCH_VERTICES, 1},
    {GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, 1},
    {GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, 1},
    {GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, 1},
    {GL_MAX_TESS_GEN_LEVEL, 1},
    {GL_MAX_TESS_PATCH_COMPONENTS, 1},

    // Tessellation evaluation shaders
    {GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, 1},
    {GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, 1},

    // Vertex shaders
    {GL_MAX_VERTEX_ATTRIBS, 1},
    {GL_MAX_VERTEX_OUTPUT_COMPONENTS, 1},
    {GL_MAX_VERTEX_UNIFORM_VECTORS, 1},

    // Textures
    {GL_MAX_3D_TEXTURE_SIZE, 1},
    {GL_MAX_ARRAY_TEXTURE_LAYERS, 1},
    {GL_MAX_CUBE_MAP_TEXTURE_SIZE, 1},
    {GL_MAX_RECTANGLE_TEXTURE_SIZE, 1},
    {GL_MAX_RENDERBUFFER_SIZE, 1},
    {GL_MAX_TEXTURE_BUFFER_SIZE, 1},
    {GL_MAX_TEXTURE_LOD_BIAS, 1},
    {GL_MAX_TEXTURE_SIZE, 1},
    {GL_NUM_COMPRESSED_TEXTURE_FORMATS, 1},

    // Transformation state
    {GL_MAX_CLIP_DISTANCES, 1},
    {GL_MAX_VIEWPORT_DIMS, 2},
    {GL_MAX_VIEWPORTS, 1},
    {GL_VIEWPORT_BOUNDS_RANGE, 2},

    // Vertex arrays
    {GL_MAX_ELEMENT_INDEX, 1},
    {GL_MAX_ELEMENTS_INDICES, 1},
    {GL_MAX_ELEMENTS_VERTICES, 1}
});

context::context()
{
}

context::~context()
{
    for(auto& pair: param_cache)
    {
        delete [] (char*)pair.second;
    }
}

const std::string& context::get_vendor_name() const { return vendor; }
const std::string& context::get_renderer() const { return renderer; }

GLint64 context::operator[](GLenum pname) const
{
    return get(pname);
}

GLint64 context::get(GLenum pname) const
{
    GLint64 value;
    get(pname, 1, &value);
    return value;
}

glm::ivec2 context::get2(GLenum pname) const
{
    glm::ivec2 value;
    get(pname, 2, &value);
    return value;
}

glm::ivec3 context::get3(GLenum pname) const
{
    glm::ivec3 value;
    get(pname, 3, &value);
    return value;
}

glm::ivec4 context::get4(GLenum pname) const
{
    glm::ivec4 value;
    get(pname, 4, &value);
    return value;
}

GLint64 context::get(GLenum pname, GLuint index) const
{
    GLint64 value;
    get(pname, 1, &value, &index);
    return value;
}

glm::ivec2 context::get2(GLenum pname, GLuint index) const
{
    glm::ivec2 value;
    get(pname, 2, &value, &index);
    return value;
}

glm::ivec3 context::get3(GLenum pname, GLuint index) const
{
    glm::ivec3 value;
    get(pname, 3, &value, &index);
    return value;
}

glm::ivec4 context::get4(GLenum pname, GLuint index) const
{
    glm::ivec4 value;
    get(pname, 4, &value, &index);
    return value;
}

static void get_gl_value(
    GLenum pname,
    size_t size,
    void* value,
    const GLuint* index
){
    if(size == 1)
    {
        if(index) glGetInteger64i_v(pname, *index, (GLint64*)value);
        else glGetInteger64v(pname, (GLint64*)value);
    }
    else
    {
        if(index) glGetIntegeri_v(pname, *index, (GLint*)value);
        else glGetIntegerv(pname, (GLint*)value);
    }
}

void context::get(
    GLenum pname,
    size_t size,
    void* value,
    const GLuint* index
) const {
    auto it = param_cache.find(pname);

    // Not cached
    if(it == param_cache.end())
    {
        auto it2 = cacheable_params.find(pname);
        // No record of this parameter; assume the user knows what he is doing
        // but don't cache.
        if(it2 == cacheable_params.end())
        {
            get_gl_value(pname, size, value, index);
            return;
        }
        else if(it2->second == size)
        {
            if(index)
                throw std::runtime_error(
                    "Index given for non-indexed glGet parameter"
                );

            void* cache_entry;
            switch(size)
            {
            case 1:
                cache_entry = new char[sizeof(GLint64)];
                break;
            default:
                cache_entry = new char[sizeof(GLint) * size];
                break;
            }
            get_gl_value(pname, size, cache_entry, nullptr);
            it = param_cache.emplace(pname, cache_entry).first;
        }
        else throw std::runtime_error(
            "Wrong size for glGet parameter: got " + std::to_string(size)
            + ", expected " + std::to_string(it2->second)
        );
    }

    // Was cached, just copy.
    switch(size)
    {
    case 1:
        memcpy(value, it->second, sizeof(GLint64));
        break;
    default:
        memcpy(value, it->second, size * sizeof(GLint));
        break;
    }
}

void context::init()
{
    vendor = std::string((const char*)glGetString(GL_VENDOR));
    renderer = std::string((const char*)glGetString(GL_RENDERER));
}
