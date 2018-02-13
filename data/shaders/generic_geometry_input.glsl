in VERTEX_OUT {
    vec3 position;
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
} g_in[];

out VERTEX_OUT {
    vec3 position;
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
} g_out;
