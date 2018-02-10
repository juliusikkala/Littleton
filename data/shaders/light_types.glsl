#include "constants.glsl"

struct point_light
{
    vec3 color;
    vec3 position;
};

struct directional_light
{
    vec3 color;
    vec3 direction;
};

struct spotlight
{
    vec3 color;
    vec3 position;
    vec3 direction;
    float cutoff;
    float exponent;
};

vec3 point_light_diffuse(point_light l, vec3 pos, vec3 normal)
{
    vec3 dir = l.position - pos;
    float dist = length(dir);
    dir/=dist;
    float ctheta = clamp(dot(normal, dir), 0, 1);
    return l.color*ctheta/(dist*dist);
}

vec3 directional_light_diffuse(directional_light l, vec3 normal)
{
    vec3 dir = l.direction;
    float ctheta = clamp(dot(normal, -dir), 0, 1);
    return l.color*ctheta;
}

vec3 spotlight_diffuse(spotlight l, vec3 pos, vec3 normal)
{
    vec3 dir = l.position - pos;
    float dist = length(dir);
    dir/=dist;

    float cutoff = dot(dir, -l.direction);
    cutoff = cutoff > l.cutoff ?
        1-pow(1-(cutoff-l.cutoff)/(1-l.cutoff), l.exponent) :
        0;

    float ctheta = clamp(dot(normal, dir), 0, 1)*cutoff;
    return l.color*ctheta/(dist*dist);
}
