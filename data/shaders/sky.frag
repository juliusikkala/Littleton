#version 330 core
#include "constants.glsl"

in vec3 pos;
in vec2 uv;

uniform sampler2D in_color;

#include "depth.glsl"
out vec4 out_color;

uniform vec3 rayleigh_coef;
uniform float inv_rayleigh_scale_height;
uniform float inv_mie_scale_height;
uniform float mie_coef;
uniform float mie_anisotropy;
uniform float ground_radius;
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

bool intersect_sphere(
    vec3 pos,
    vec3 dir,
    vec3 origin,
    out float t0,
    out float t1
){
    vec3 L = pos - origin;
    float a = dot(dir, dir);
    float b = 2*dot(dir, L);
    float c = dot(L, L) - atmosphere_radius2;

    if(!quadratic(a, b, c, t0, t1)) return false;
    if(t1 < 0) return false;
    if(t0 < 0) t0 = 0;

    return true;
}

#ifndef VIEW_SAMPLES
#define VIEW_SAMPLES 8
#endif

#ifndef LIGHT_SAMPLES
#define LIGHT_SAMPLES 4
#endif

vec4 sky_color(vec3 view_dir, float t0, float t1)
{
    vec3 x0 = t0 * view_dir;
    vec3 x1 = t1 * view_dir;
    float segment_length = (t1 - t0) / VIEW_SAMPLES;
    vec3 segment = view_dir * segment_length;
    vec3 x = x0 + segment * 0.5f;

    float mu = dot(sun_direction, view_dir);
    float mu2 = mu * mu;

    float r_optical_depth = 0;
    float m_optical_depth = 0;

    float g = mie_anisotropy;
    float g2 = g * g;

    float r_phase = 3.0f/(16.0f*PI)*(1.0f + mu2);
    float m_phase = 3.0f/(8.0f*PI)*(1.0f - g2) * (1.0f + mu2) /
        ((2.0f + g2)*pow(1 + g2 - 2 * g * mu, 1.5f));

    vec3 r_sum = vec3(0);
    vec3 m_sum = vec3(0);

    // View samples
    for(int i = 0; i < VIEW_SAMPLES; ++i)
    {
        float h = distance(x, origin) - ground_radius;
        float r_h = exp(-h * inv_rayleigh_scale_height) * segment_length;
        float m_h = exp(-h * inv_mie_scale_height) * segment_length;
        r_optical_depth += r_h;
        m_optical_depth += m_h;

        // Ray from sample position to edge of atmosphere towards light
        float lt0, lt1;
        intersect_sphere(x, sun_direction, origin, lt0, lt1);
        float light_segment_length = (lt1 - lt0) * (1.0f / LIGHT_SAMPLES);
        vec3 light_segment = sun_direction * light_segment_length;
        vec3 lx = x + light_segment * 0.5f;
        float lr_optical_depth = 0;
        float lm_optical_depth = 0;
        int j = 0;
        for(j = 0; j < LIGHT_SAMPLES; ++j)
        {
            float lh = distance(lx, origin) - ground_radius;
            lr_optical_depth += exp(-lh * inv_rayleigh_scale_height) * light_segment_length;
            lm_optical_depth += exp(-lh * inv_mie_scale_height) * light_segment_length;
            lx += light_segment;
        }
        vec3 T = rayleigh_coef * (r_optical_depth + lr_optical_depth) +
                 mie_coef * 1.1f * (m_optical_depth + lm_optical_depth);
        vec3 attenuation = exp(-T);
        r_sum += attenuation * r_h;
        m_sum += attenuation * m_h;

        x += segment;
    }
    vec3 color = (r_sum * rayleigh_coef * r_phase + m_sum * mie_coef * m_phase) * sun_color * 10;

    vec3 T = rayleigh_coef * (r_optical_depth) +
             mie_coef * 1.1f * (m_optical_depth);
    vec3 attenuation = exp(-T);

    return vec4(color, 1-attenuation.r);
}

void main(void)
{
    vec3 dir = normalize(pos);
    float t0, t1;

    float d = get_linear_depth();
    if(intersect_sphere(vec3(0), dir, origin, t0, t1))
    {
        if(t0 * dir.z < d) discard;
        t1 = max(t1 * dir.z, d) / dir.z;
        out_color = sky_color(dir, t0, t1);
    }
    else discard;
}

