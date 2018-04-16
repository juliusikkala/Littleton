#define _USE_MATH_DEFINES
#include "helpers.hh"
#include <cstdio>
#include <cmath>
#include <stdexcept>
#include <glm/gtc/random.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glm/gtx/transform.hpp>

std::string read_text_file(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "rb");

    if(!f)
    {
        throw std::runtime_error("Unable to open " + path);
    }

    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = new char[sz];
    if(fread(data, 1, sz, f) != sz)
    {
        delete [] data;
        throw std::runtime_error("Unable to read " + path);
    }
    fclose(f);
    std::string ret(data, sz);

    delete [] data;
    return ret;
}

bool read_binary_file(const std::string& path, uint8_t*& data, size_t& bytes)
{
    FILE* f = fopen(path.c_str(), "rb");

    if(!f) return false;

    fseek(f, 0, SEEK_END);
    bytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = new uint8_t[bytes];
    if(fread(data, 1, bytes, f) != bytes)
    {
        delete [] data;
        bytes = 0;
        return false;
    }
    fclose(f);

    return true;
}

bool write_binary_file(const std::string& path, const uint8_t* data, size_t bytes)
{
    FILE* f = fopen(path.c_str(), "wb");

    if(!f) return false;

    if(fwrite(data, 1, bytes, f) != bytes) return false;
    fclose(f);

    return true;
}

void decompose_matrix(
    const glm::mat4& transform,
    glm::vec3& translation,
    glm::vec3& scaling,
    glm::quat& orientation
){
    translation = transform[3];
    scaling = glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );
    orientation = glm::quat(glm::mat4(
        transform[0]/scaling.x,
        transform[1]/scaling.y,
        transform[2]/scaling.z,
        glm::vec4(0,0,0,1)
    ));
}

glm::vec3 get_matrix_translation(const glm::mat4& transform)
{
    return transform[3];
}

glm::vec3 get_matrix_scaling(const glm::mat4& transform)
{
    return glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );
}

glm::quat get_matrix_orientation(const glm::mat4& transform)
{
    return glm::quat(glm::mat4(
        glm::normalize(transform[0]),
        glm::normalize(transform[1]),
        glm::normalize(transform[2]),
        glm::vec4(0,0,0,1)
    ));
}

void decompose_perspective(
    const glm::mat4& perspective,
    float& near,
    float& far,
    float& fov,
    float& aspect
){
    float a = perspective[2][2];
    float b = perspective[3][2];
    float c = perspective[0][0];
    float d = perspective[1][1];

    near = fabs(-b/(1-a));
    far = fabs(b/(1+a));
    fov = 2*atan(1/d);
    aspect = d/c;
}

glm::quat rotate_towards(
    glm::quat orig,
    glm::quat dest,
    float angle_limit
){
    angle_limit = glm::radians(angle_limit);

    float cos_theta = dot(orig, dest);
    if(cos_theta > 0.999999f)
    {
        return dest;
    }

    if(cos_theta < 0)
    {
        orig *= -1.0f;
        cos_theta *= -1.0f;
    }

    float theta = acos(cos_theta);
    if(theta < angle_limit) return dest;
    return glm::mix(orig, dest, angle_limit/theta);
}

glm::quat quat_lookat(
    glm::vec3 dir,
    glm::vec3 up,
    glm::vec3 forward
){
    dir = normalize(dir);
    glm::quat towards = glm::rotation(forward, dir);
    glm::vec3 right = glm::cross(dir, up);

    glm::quat fix_up = glm::rotation(
        towards * glm::vec3(0,1,0),
        cross(right, dir)
    );

    return glm::normalize(fix_up * towards);
}

bool solve_quadratic(float a, float b, float c, float& x0, float& x1)
{
    float D = b * b - 4 * a * c;
    float sD = sqrt(D) * sign(a);
    float denom = -0.5f/a;
    x0 = (b + sD) * denom;
    x1 = (b - sD) * denom;
    return !std::isnan(sD);
}

bool intersect_sphere(
    glm::vec3 pos,
    glm::vec3 dir,
    glm::vec3 origin,
    float radius,
    float& t0,
    float& t1
){
    glm::vec3 L = pos - origin;
    float a = glm::dot(dir, dir);
    float b = 2*glm::dot(dir, L);
    float c = glm::dot(L, L) - radius * radius;

    if(!solve_quadratic(a, b, c, t0, t1)) return false;
    if(t1 < 0) return false;
    if(t0 < 0) t0 = 0;

    return true;
}

unsigned next_power_of_two(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

unsigned factorize(unsigned n)
{
    // Divisible by two
    if((n&1)==0) return 2;

    unsigned last = floor(sqrt(n));
    for(unsigned i = 3; i <= last; ++i)
        if((n % i) == 0) return i;

    return 0;
}

static glm::vec2 circle_projection_range(
    glm::vec2 dir,
    float r,
    float p,
    float big
){
    float d2 = glm::dot(dir, dir);
    float r2 = r * r;

    if(d2 <= r2) { return glm::vec2(-1, 1) * big; }

    float len = sqrt(d2 - r2);
    glm::vec2 n = dir / dir.y;

    glm::vec2 h(-n.y, n.x);
    h *= r / len;

    glm::vec2 up = n + h;
    float top = up.x / fabs(up.y) * p;

    glm::vec2 down = n - h;
    float bottom = down.x / fabs(down.y) * p;

    if(dir.x > 0 && dir.y <= r)
    {
        bottom = big;
        if(dir.y <= 0) top = -top;
    }

    if(dir.x < 0 && dir.y <= r)
    {
        top = -big;
        if(dir.y <= 0) bottom = -bottom;
    }

    return glm::vec2(top, bottom);
}

glm::mat4 sphere_projection_quad_matrix(
    glm::vec3 pos,
    float r,
    float near,
    float far,
    bool use_near_radius,
    float big
){

    float d = -pos.z;

    if(use_near_radius) d = glm::max(d - r, near);
    else d = glm::min(d + r, far);

    glm::vec2 w = circle_projection_range(
        glm::vec2(pos.x, -pos.z),
        r, d, big
    );
    glm::vec2 h = circle_projection_range(
        glm::vec2(pos.y, -pos.z),
        r, d, big
    );

    glm::vec2 center = glm::vec2(w.x + w.y, h.x + h.y) / 2.0f;
    glm::vec2 scale = glm::vec2(fabs(w.y - w.x), fabs(h.y - h.x)) / 2.0f;

    return glm::translate(glm::vec3(center, -d)) *
           glm::scale(glm::vec3(scale, 0));
}

template<typename T, class F>
void mitchell_best_candidate(
    std::vector<T>& samples,
    F&& sample_generator,
    unsigned candidate_count,
    unsigned count
){
    if(count < samples.size()) return;

    samples.reserve(count);
    count -= samples.size();

    while(count--)
    {
        T farthest = T(0);
        float farthest_distance = 0;

        for(unsigned i = 0; i < candidate_count; ++i)
        {
            T candidate = sample_generator();
            float candidate_distance = INFINITY;

            for(const T& sample: samples)
            {
                float dist = glm::distance(candidate, sample);
                if(dist < candidate_distance) candidate_distance = dist;
            }

            if(candidate_distance > farthest_distance)
            {
                farthest_distance = candidate_distance;
                farthest = candidate;
            }
        }

        samples.push_back(farthest);
    }
}

void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [r](){return glm::diskRand(r);},
        candidate_count,
        count
    );
}

void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float w,
    float h,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [w, h](){
            return glm::linearRand(glm::vec2(-w/2, -h/2), glm::vec2(w/2, h/2));
        },
        candidate_count,
        count
    );
}

void mitchell_best_candidate(
    std::vector<glm::vec3>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [r](){return glm::ballRand(r);},
        candidate_count,
        count
    );
}

std::vector<glm::vec2> grid_samples(
    unsigned w,
    unsigned h,
    float step
){
    std::vector<glm::vec2> samples;
    samples.resize(w*h);

    glm::vec2 start(
        (w-1)/-2.0f,
        (h-1)/-2.0f
    );

    for(unsigned i = 0; i < h; ++i)
        for(unsigned j = 0; j < w; ++j)
            samples[i*w+j] = start + glm::vec2(i, j) * step;

    return samples;
}

std::vector<float> generate_gaussian_kernel(
    int radius,
    float sigma
){
    std::vector<float> result;
    result.reserve(radius * 2 + 1);

    for(int i = -radius; i <= radius; ++i)
    {
        float f = i/sigma;
        float weight = exp(-f*f/2.0f)/(sigma * sqrt(2*M_PI));
        result.push_back(weight);
    }
    return result;
}

size_t count_lines(const std::string& str)
{
    return 1 + std::count(str.begin(), str.end(), '\n');
}

std::string add_line_numbers(const std::string& src)
{
    std::stringstream in(src);
    std::stringstream out;
    size_t line_count = count_lines(src);
    size_t number_width = 1 + floor(log10(line_count));
    std::string line;

    unsigned line_number = 1;
    while(std::getline(in, line, '\n'))
    {
        out << "| " << std::setw(number_width) << line_number << " |"
            << line << std::endl;

        line_number++;
    }

    return out.str();
}

GLint internal_format_to_external_format(GLint internal_format)
{
    switch(internal_format)
    {
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_COMPRESSED_RED:
    case GL_COMPRESSED_RED_RGTC1:
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
        return GL_RED;
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_COMPRESSED_RG:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
        return GL_RG;
    case GL_RGB:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_COMPRESSED_RGB:
    case GL_COMPRESSED_SRGB:
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        return GL_RGB;
    case GL_BGR:
        return GL_BGR;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_COMPRESSED_RGBA:
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_SRGB_ALPHA:
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        return GL_RGBA;
    case GL_BGRA:
        return GL_BGRA;
    case GL_RED_INTEGER:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
        return GL_RED_INTEGER;
    case GL_RG_INTEGER:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
        return GL_RG_INTEGER;
    case GL_RGB_INTEGER:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
    case GL_RGB10_A2UI:
        return GL_RGB_INTEGER;
    case GL_BGR_INTEGER:
        return GL_BGR_INTEGER;
    case GL_RGBA_INTEGER:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        return GL_RGBA_INTEGER;
    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1:
    case GL_STENCIL_INDEX4:
    case GL_STENCIL_INDEX8:
    case GL_STENCIL_INDEX16:
        return GL_STENCIL_INDEX;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return GL_DEPTH_COMPONENT;
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return GL_DEPTH_STENCIL;
    default:
        throw std::runtime_error(
            "Unknown internal texture format "
            + std::to_string(internal_format)
        );
    }
}

GLint internal_format_compatible_type(GLint internal_format)
{
    if(internal_format == GL_DEPTH24_STENCIL8) return GL_UNSIGNED_INT_24_8;
    return GL_UNSIGNED_BYTE;
}

unsigned internal_format_channel_count(GLint internal_format)
{
    switch(internal_format)
    {
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_COMPRESSED_RED:
    case GL_COMPRESSED_RED_RGTC1:
    case GL_COMPRESSED_SIGNED_RED_RGTC1:
    case GL_RED_INTEGER:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1:
    case GL_STENCIL_INDEX4:
    case GL_STENCIL_INDEX8:
    case GL_STENCIL_INDEX16:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return 1;
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_COMPRESSED_RG:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
    case GL_RG_INTEGER:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return 2;
    case GL_RGB:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_COMPRESSED_RGB:
    case GL_COMPRESSED_SRGB:
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
    case GL_BGR:
    case GL_RGB_INTEGER:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
    case GL_RGB10_A2UI:
    case GL_BGR_INTEGER:
        return 3;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_COMPRESSED_RGBA:
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_SRGB_ALPHA:
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
    case GL_BGRA:
    case GL_RGBA_INTEGER:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        return 4;
    default:
        throw std::runtime_error(
            "Unknown internal texture format "
            + std::to_string(internal_format)
        );
    }
}

unsigned gl_type_sizeof(GLenum type)
{
    switch(type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_HALF_FLOAT:
        return 2;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FIXED:
    case GL_FLOAT:
        return 4;
    case GL_DOUBLE:
        return 8;
    default:
        throw std::runtime_error(
            "Unknown OpenGL type " + std::to_string(type)
        );
    }
}

GLenum get_binding_name(GLenum target)
{
    switch(target)
    {
    case GL_TEXTURE_1D:
        return GL_TEXTURE_BINDING_1D;
    case GL_TEXTURE_2D:
        return GL_TEXTURE_BINDING_2D;
    case GL_TEXTURE_3D:
        return GL_TEXTURE_BINDING_3D;
    case GL_TEXTURE_1D_ARRAY:
        return GL_TEXTURE_BINDING_1D_ARRAY;
    case GL_TEXTURE_2D_ARRAY:
        return GL_TEXTURE_BINDING_2D_ARRAY;
    case GL_TEXTURE_RECTANGLE:
        return GL_TEXTURE_BINDING_RECTANGLE;
    case GL_TEXTURE_BUFFER:
        return GL_TEXTURE_BINDING_BUFFER;
    case GL_TEXTURE_CUBE_MAP:
        return GL_TEXTURE_BINDING_CUBE_MAP;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
    case GL_TEXTURE_2D_MULTISAMPLE:
        return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
    default:
        throw std::runtime_error(
            "Unknown texture target " + std::to_string(target)
        );
    }
}

