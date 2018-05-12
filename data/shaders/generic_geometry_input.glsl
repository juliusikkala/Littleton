in VERTEX_OUT {
    vec3 position;
#if defined(DIRECTIONAL_SHADOW_MAPPING) || defined(PERSPECTIVE_SHADOW_MAPPING)
    vec4 light_space_pos;
#endif
#ifdef VERTEX_NORMAL
    vec3 normal;
#ifdef VERTEX_TANGENT
    vec3 tangent;
    vec3 bitangent;
#endif
#endif
#ifdef VERTEX_UV0
    vec2 uv;
#endif
} g_in[];

out VERTEX_OUT {
    vec3 position;
#if defined(DIRECTIONAL_SHADOW_MAPPING) || defined(PERSPECTIVE_SHADOW_MAPPING)
    vec4 light_space_pos;
#endif
#ifdef VERTEX_NORMAL
    vec3 normal;
#ifdef VERTEX_TANGENT
    vec3 tangent;
    vec3 bitangent;
#endif
#endif
#ifdef VERTEX_UV0
    vec2 uv;
#endif
} g_out;
