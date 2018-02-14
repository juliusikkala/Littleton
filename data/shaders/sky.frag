#version 330 core

in vec3 pos;
in vec2 uv;

#include "depth.glsl"
out vec4 out_color;

uniform vec3 rayleigh_coef;
uniform float rayleigh_scale_height;
uniform float mie_scale_height;
uniform float mie_coef;
uniform float mie_anisotropy;
uniform float ground_radius2;
uniform float atmosphere_radius2;
uniform vec3 sun_direction;
uniform vec3 sun_color;
uniform vec3 origin;

bool quadratic(float a, float b, float c, out float x0, out float x1)
{
    float D = b * b - 4 * a * c;
    float sD = sqrt(D) * sign(a);
    float denom = -0.5f/a;
    x0 = (b + sD) * denom;
    x1 = (b - sD) * denom;
    return !isnan(sD);
}

bool intersect_sphere(vec3 dir, out float t0, out float t1)
{
    vec3 L = -origin;
    float a = dot(dir, dir);
    float b = 2*dot(dir, L);
    float c = dot(L, L) - atmosphere_radius2;

    if(!quadratic(a, b, c, t0, t1)) return false;
    if(t1 < 0) return false;
    if(t0 < 0) t0 = 0;

    return true;
}

void main(void)
{
    vec3 dir = pos;
    float t0, t1;

    float d = -get_linear_depth();
    if(intersect_sphere(dir, t0, t1))
    {
        if(t0 > d) discard;
        t1 = min(t1, d);
        float l = min((t1-t0)/2, 1);
        out_color = vec4(vec3(0.5,0.5,1), l);
    }
    else discard;
}

