#include "constants.glsl"

struct sg_lobe
{
    vec3 amplitude;
    vec3 axis;
    float sharpness;
};

vec3 sg_value(sg_lobe s, vec3 d)
{
    return s.amplitude * exp(s.sharpness * (dot(d, s.axis) - 1.0f));
}

sg_lobe sg_product(sg_lobe a, sg_lobe b)
{
    sg_lobe c;

    c.axis = (a.sharpness * a.axis + b.sharpness * b.axis)/
        (a.sharpness + b.sharpness);
    float len = length(c.axis);
    c.axis /= len;

    float sharpness = a.sharpness + b.sharpness;

    c.sharpness = sharpness * len;
    c.amplitude = a.amplitude * b.amplitude * exp(sharpness * (len - 1.0f));

    return c;
}

vec3 sg_integral(sg_lobe a)
{
    return 2.0f * PI * (a.amplitude / a.sharpness) *
        (1.0f - exp(-2.0f * a.sharpness));
}

vec3 sg_approx_integral(sg_lobe a)
{
    return 2.0f * PI * (a.amplitude / a.sharpness);
}

vec3 sg_inner_product(sg_lobe a, sg_lobe b)
{
    float len = length(a.sharpness * a.axis + b.sharpness * b.axis);
    return (2.0f * PI *
        exp(len - a.sharpness - b.sharpness) * a.amplitude * b.amplitude *
        (1.0f - exp(-2.0f * len))) / len;
}

// Hill 16
vec3 sg_approx_irradiance(sg_lobe s, vec3 normal)
{
    float cos_theta = dot(s.axis, normal);

    const float c0 = 0.36f;
    const float c1 = 1.0f / (4.0f * 0.36f);
    
    float eml = exp(-s.sharpness);
    float em2l = eml * eml;
    float rl = 1.0f / s.sharpness;

    float scale = 1.0f + 2.0f * em2l - rl;
    float bias = (eml - em2l) * rl - em2l;

    float x = sqrt(1.0f - scale);
    float x0 = c0 * cos_theta;
    float x1 = c1 * x;

    float n = x0 + x1;

    float y = clamp(cos_theta, 0.0f, 1.0f);
    if(abs(x0) <= x1)
        y = n * n / x;

    float result = scale * y + bias;

    return result * sg_approx_integral(s);
}

