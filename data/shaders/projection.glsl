#include "constants.glsl"
uniform vec2 projection_info;

vec3 unproject_position(float linear_depth, vec2 uv)
{
    return vec3((0.5f-uv) * projection_info * linear_depth, linear_depth);
}

vec2 project_position(vec3 pos)
{
    return 0.5f - pos.xy / (projection_info * pos.z);
}

vec3 calculate_view_ray(vec2 uv)
{
    return vec3((uv-0.5f) * projection_info, -1.0f);
}

vec2 horizon(vec3 o, vec3 d, float near)
{
    if(d.z < 0) return 0.5f - d.xy / (projection_info * d.z);
    else
    {
        vec3 h = o + d * ((near - o.z)/(d.z));
        return 0.5f - h.xy / (projection_info * h.z);
    }
}

// Calculates t in project_position(o + t*d) == uv
float calculate_ray_length(vec3 o, vec3 d, vec2 uv)
{
    vec2 v = (uv-0.5f) * projection_info;
    v.x = -v.x;
    return -dot(o.xy, v.yx)/dot(d.xy, v.yx);
}

vec2 projected_ray_direction(vec3 o, vec3 d)
{
    return (o.xy*d.z - o.z*d.xy)/projection_info;
}

vec2 project_lambert_azimuthal_equal_area(vec3 normal)
{
    vec3 n = normalize(normal);
    return inversesqrt(2.0f+2.0f*n.z)*n.xy;
}

vec3 unproject_lambert_azimuthal_equal_area(vec2 n2)
{
    n2 *= SQRT2;
    float d = dot(n2, n2);
    float f = sqrt(2.0f - d);
    return vec3(f*n2, 1.0f - d);
}
