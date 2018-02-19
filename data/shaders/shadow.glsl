struct shadow_map
{
    sampler2DShadow map;
    mat4 mvp;
    float min_bias;
    float max_bias;
};

float get_shadow_bias(
    vec3 normal,
    vec3 light_dir,
    float min_bias,
    float max_bias
){
    return max(max_bias * (1.0f - dot(normal, light_dir)), min_bias);
}

float shadow_coef(
    in sampler2DShadow shadow_map,
    vec4 light_space_pos,
    float bias
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    pos = pos * 0.5f + 0.5f;
    if(pos.z > 1.0f) return 1.0f;

    return texture(shadow_map, vec3(pos.xy, pos.z-bias));
}
