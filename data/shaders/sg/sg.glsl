#include "constants.glsl"
#include "brdf.glsl"

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

sg_lobe sg_distribution(vec3 dir, float a2)
{
    sg_lobe distribution;
    distribution.axis = dir;
    distribution.sharpness = 2.0f / a2;
    distribution.amplitude = vec3(1.0f / (PI * a2));
    return distribution;
}

sg_lobe sg_warp_distribution(sg_lobe ndf, vec3 view)
{
    sg_lobe warp;
    warp.axis = reflect(-view, ndf.axis);
    warp.amplitude = ndf.amplitude;
    warp.sharpness = ndf.sharpness / (4.0f * max(dot(ndf.axis, view), 0.0001f));
    return warp;
}

struct asg_lobe
{
    vec3 amplitude;
    mat3 basis;
    vec2 sharpness;
};

vec3 asg_value(asg_lobe s, vec3 d)
{
    float st = clamp(dot(s.basis[2], d), 0.0f, 1.0f);
    vec2 db = vec2(dot(d, s.basis[0]), dot(d, s.basis[1]));
    return s.amplitude * st * exp(-dot(s.sharpness, db * db));
}

vec3 asg_sg_convolution(asg_lobe a, sg_lobe s) {
    float nu = s.sharpness * 0.5f;

    asg_lobe convolve;
    convolve.basis = a.basis;
    convolve.sharpness = nu * a.sharpness / (vec2(nu) + a.sharpness);

    convolve.amplitude = vec3(PI * inversesqrt(
        (nu + a.sharpness.x) * (nu + a.sharpness.y)
    ));

    vec3 res = asg_value(convolve, s.axis);
    return res * s.amplitude * a.amplitude;
}

asg_lobe asg_warp_distribution(sg_lobe ndf, vec3 view)
{
    asg_lobe warp;

    warp.basis[2] = reflect(-view, ndf.axis);
    warp.basis[0] = normalize(cross(ndf.axis, warp.basis[2]));
    warp.basis[1] = normalize(cross(warp.basis[2], warp.basis[0]));

    float o = max(dot(view, ndf.axis), 0.0001f);

    warp.sharpness.y = ndf.sharpness * 0.125f;
    warp.sharpness.x = warp.sharpness.y / (o * o);

    warp.amplitude = ndf.amplitude;

    return warp;
}

float inv_ggx_v1(float cos_h, float a2)
{
    return cos_h + sqrt(a2 + (1 - a2) * cos_h * cos_h);
}

vec3 asg_approx_specular(
    sg_lobe light,
    vec3 surface_color,
    vec3 view_dir,
    vec3 normal,
    float roughness,
    float f0,
    float metallic
){
    float a2 = roughness * roughness;
    a2 *= a2;
    sg_lobe ndf = sg_distribution(normal, a2);
    asg_lobe warped_ndf = asg_warp_distribution(ndf, view_dir);

    vec3 res = asg_sg_convolution(warped_ndf, light);

    vec3 warp_dir = warped_ndf.basis[2];
    float ndotl = clamp(dot(normal, warp_dir), 0.0f, 1.0f);
    float ndotv = clamp(dot(normal, view_dir), 0.0f, 1.0f);
    vec3 h = normalize(warp_dir + view_dir);

    res /= inv_ggx_v1(a2, ndotl) * inv_ggx_v1(a2, ndotv);

    vec3 f0_m = mix(vec3(f0), surface_color, metallic);

    // Fresnel
    float cos_d = clamp(dot(warp_dir, h), 0.0f, 1.0f);
    res *= fresnel_schlick(cos_d, f0_m);

    // Cosine term
    res *= ndotl;

    return max(res, 0.0f);
}
