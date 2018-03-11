// Moment Shadow Mapping
#include "constants.glsl"

struct shadow_map
{
    sampler2D map;
    mat4 mvp;
};

// Adapted to GLSL from Peters Christoph's GDC 2016 slides
// "Rendering Antialiased Shadows"
float msm_shadow_intensity(vec4 b, float depth)
{
    float L32D22 = fma(-b.x, b.y, b.z);
    float D22 = fma(-b.x, b.x, b.y);
    float SquaredDepthVariance = fma(-b.y, b.y, b.w);
    float D33D22 = dot(
        vec2(SquaredDepthVariance, -L32D22),
        vec2(D22, L32D22)
    );
    float InvD22 = 1.0f/D22;
    float L32 = L32D22 * InvD22;
    vec3 z;
    z.x = depth;
    vec3 c = vec3(1.0f, z.x, z.x*z.x);
    c.y -= b.x;
    c.z -= b.y + L32*c.y;
    c.y *= InvD22;
    c.z *= D22/D33D22;
    c.y -= L32*c.z;
    c.x -= dot(c.yz, b.xy);
    float InvC2 = 1.0f / c.z;
    float p = c.y * InvC2;
    float q = c.x * InvC2;
    float r = sqrt((p * p * 0.25f) - q);
    z.y = -p*0.5f-r;
    z.z = -p*0.5f+r;
    vec4 Switch =
        (z.z < z.x) ? vec4(z.y, z.x, 1.0f, 1.0f) : (
        (z.y < z.x) ? vec4(z.x, z.y, 0.0f, 1.0f) :
        vec4(0.0f));
    float Quotient = (Switch.x * z.z - b.x * (Switch.x + z.z) + b.y)
        / ((z.z - Switch.y)*(z.x - z.y));

    return clamp((Switch.z + Switch.w * Quotient)*1.03f, 0.0f, 1.0f);
}

float shadow_coef(
    in shadow_map sm,
    vec4 light_space_pos,
    vec3 normal,
    vec3 light_dir
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    if(abs(pos.z) > 1.0f) return 1.0f;

    pos.xy = pos.xy * 0.5f + 0.5;

    mat4 q = mat4(
        -1.0f/3.0f, 0.0f, -0.75f, 0.0f,
        0.0f, 0.125f, 0.0f, -0.125f,
        SQRT3, 0.0f, 0.75f*SQRT3, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    );
    vec4 m = q * (texture(sm.map, pos.xy) - vec4(0.5f, 0.0f, 0.5f, 0.0f));
    float alpha = 6.1e-5;
    vec4 b = mix(m, vec4(0.0f, 0.63f, 0.0f, 0.63f), alpha);

    return 1.0f - msm_shadow_intensity(b, pos.z);
}

