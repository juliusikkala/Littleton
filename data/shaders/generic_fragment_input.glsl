in VERTEX_OUT {
    vec3 position;
#ifdef SHADOW_MAPPING
    vec4 light_space_pos;
#endif
#ifdef VERTEX_NORMAL
    vec3 normal;
#ifdef VERTEX_TANGENT
    vec3 tangent;
    vec3 bitangent;
#endif
#endif
#ifdef VERTEX_UV
    vec2 uv;
#endif
} f_in;
