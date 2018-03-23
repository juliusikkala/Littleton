#include "camera.hh"
#include "helpers.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

camera::camera(transformable_node* parent): transformable_node(parent) {}
camera::~camera() {}

void camera::perspective(float fov, float aspect, float near)
{
    fov = glm::radians(fov);
    projection = glm::infinitePerspective(fov, aspect, near);
    clip_info = glm::vec3(near, -1, 1);
    projection_info = glm::vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
    tan_fov = tan(fov * 0.5f);
    inverse_cos_fov = glm::vec2(
        1.0f/cos(fov * 0.5f),
        1.0f/cos(atan(tan_fov * aspect))
    );
    this->near = near;
    this->far = far;
    this->aspect = aspect;
}

void camera::perspective(float fov, float aspect, float near, float far)
{
    fov = glm::radians(fov);
    projection = glm::perspective(fov, aspect, near, far);
    clip_info = glm::vec3(near * far, near - far, near + far);
    projection_info = glm::vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
    tan_fov = tan(fov * 0.5f);
    inverse_cos_fov = glm::vec2(
        1.0f/cos(atan(tan_fov * aspect)),
        1.0f/cos(fov * 0.5f)
    );
    this->near = near;
    this->far = far;
    this->aspect = aspect;
}

glm::mat4 camera::get_projection() const
{
    return projection;
}

glm::vec3 camera::get_clip_info() const
{
    return clip_info;
}

glm::vec2 camera::get_projection_info() const
{
    return projection_info;
}

// Using method described in http://www.lighthouse3d.com/tutorials/view-frustum-culling/radar-approach-testing-points/
bool camera::sphere_is_visible(glm::vec3 pos, float r) const
{
    glm::vec3 view = pos;
    glm::vec3 pc(view.x, view.y, -view.z);

    if(pc.z > far + r || pc.z < near - r) return false;
    
    float d = r * inverse_cos_fov.y;
    pc.z *= tan_fov;
    if(pc.y > pc.z + d || pc.y < -pc.z - d) return false;

    pc.z *= aspect;
    d = r * inverse_cos_fov.x;
    if(pc.x > pc.z + d || pc.x < -pc.z - d) return false;

    return true;
}

static glm::vec2 axis_extent(float near, glm::vec2 d, float r)
{
    glm::vec2 n = (d / glm::max(d.y, 0.0001f)) * near;

    float D = glm::dot(d, d) - r * r;
    if(D > 0)
    {
        float rt = r / sqrt(D);

        glm::vec2 rt_v(-n.y, n.x);
        rt_v*=rt;

        glm::vec2 up = n + rt_v;
        glm::vec2 down = n - rt_v;

        return glm::vec2(
            up.x / glm::max(up.y, 0.0001f),
            down.x / glm::max(down.y, 0.0001f)
        );
    }
    else return glm::vec2(-INFINITY, INFINITY);
}

glm::vec4 camera::sphere_extent(glm::vec3 pos, float r) const
{
    glm::vec2 w = axis_extent(
        near,
        glm::vec2(pos.x, -pos.z),
        r
    )/(tan_fov*aspect);

    glm::vec2 h = axis_extent(
        near,
        glm::vec2(pos.y, -pos.z),
        r
    )/tan_fov;

    return glm::clamp(
        glm::vec4(w.x, h.x, w.y, h.y),
        glm::vec4(-1.0f),
        glm::vec4(1.0f)
    );
}

glm::vec2 camera::pixels_per_unit(glm::uvec2 target_size) const
{
    glm::vec3 top(1,1,-1);
    glm::vec3 bottom(-1,-1,-1);
    glm::vec4 top_projection = projection * glm::vec4(top, 1);
    top_projection /= top_projection.w;
    glm::vec4 bottom_projection = projection * glm::vec4(bottom, 1);
    bottom_projection /= bottom_projection.w;

    return 0.5f * glm::vec2(
        top_projection.x - bottom_projection.x,
        top_projection.y - bottom_projection.y
    ) * glm::vec2(target_size);
}
