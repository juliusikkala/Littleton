#include "constants.glsl"

vec3 fresnel_schlick(float cos_d, vec3 f0)
{
    return f0 + (1.0f - f0) * pow(1.0f - cos_d, 5.0f);
}

// This is pre-divided by 4.0f*cos_l*cos_v (normally, that would be in the
// nominator, but it would be divided out later anyway.)
// Uses Schlick-GGX approximation for the geometry shadowing equation.
float geometry_smith(float cos_l, float cos_v, float k)
{
    float mk = 1.0f - k;
    return 0.25f / ((cos_l * mk + k) * (cos_v * mk + k));
}

float distribution_ggx(float cos_h, float a)
{
    float a2 = a * a;
    float denom = cos_h * cos_h * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * denom * denom);
}

vec3 brdf(
    vec3 light_color,
    vec3 light_dir,
    vec3 surface_color,
    vec3 view_dir,
    vec3 normal,
    float roughness,
    float f0,
    float metallic
){
    vec3 h = normalize(view_dir + light_dir);
    float cos_l = max(dot(normal, light_dir), 0.0f);
    float cos_v = max(dot(normal, view_dir), 0.0f);
    float cos_h = max(dot(normal, h), 0.0f);
    float cos_d = clamp(dot(view_dir, h), 0.0f, 1.0f);

    vec3 radiance = light_color;
    vec3 f0_m = mix(vec3(f0), surface_color, metallic);

    float k = roughness + 1.0f;
    k = k * k / 8.0f;

    vec3 fresnel = fresnel_schlick(cos_d, f0_m);
    float geometry = geometry_smith(cos_l, cos_v, k);
    float distribution = distribution_ggx(cos_h, roughness);

    vec3 specular = fresnel * geometry * distribution;

    vec3 kd = (vec3(1.0f) - fresnel) * (1.0f - metallic);

    return (kd * surface_color + specular * PI) * radiance * cos_l;
    //return (1-pow(1.0f - cos_d, 5.0f)) * radiance * cos_l;
}
