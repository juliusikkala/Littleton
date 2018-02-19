in VERTEX_OUT {
    vec3 position;
#if MAX_SHADOW_MAP_COUNT > 0
    vec4 shadow_pos[MAX_SHADOW_MAP_COUNT];
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
