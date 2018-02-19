#include "constants.glsl"
#include "brdf.glsl"

struct point_light
{
    vec3 color;
    vec3 position;
    int shadow_map_index; // negative for no shadow map
};

struct directional_light
{
    vec3 color;
    vec3 direction;
    int shadow_map_index; // negative for no shadow map
};

struct spotlight
{
    vec3 color;
    vec3 position;
    vec3 direction;
    float cutoff;
    float exponent;
    int shadow_map_index; // negative for no shadow map
};

vec3 calc_point_light(
    point_light l,
    vec3 pos,
    vec3 surface_color,
    vec3 view_dir,
    vec3 normal,
    float roughness,
    float f0,
    float metallic
){
    vec3 dir = l.position - pos;
    float dist = length(dir);
    dir/=dist;

    return brdf(
        l.color/(dist*dist),
        dir,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
}

vec3 calc_directional_light(
    directional_light l,
    vec3 surface_color,
    vec3 view_dir,
    vec3 normal,
    float roughness,
    float f0,
    float metallic
){
    return brdf(
        l.color,
        -l.direction,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
}

vec3 calc_spotlight(
    spotlight l,
    vec3 pos,
    vec3 surface_color,
    vec3 view_dir,
    vec3 normal,
    float roughness,
    float f0,
    float metallic
){
    vec3 dir = l.position - pos;
    float dist = length(dir);
    dir/=dist;

    float cutoff = dot(dir, normalize(-l.direction));
    cutoff = cutoff > l.cutoff ?
        1.0f-pow((1.0f-cutoff)/(1.0f-l.cutoff), l.exponent) :
        0.0f;

    return brdf(
        l.color*cutoff/(dist*dist),
        dir,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
}
