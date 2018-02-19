layout(location = VERTEX_POSITION) in vec3 v_vertex;

out VERTEX_OUT {
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
} v_out;

#ifdef VERTEX_NORMAL
layout(location = VERTEX_NORMAL) in vec3 v_normal;

#ifdef VERTEX_TANGENT
layout(location = VERTEX_TANGENT) in vec4 v_tangent;
#endif
#endif

#ifdef VERTEX_UV
layout(location = VERTEX_UV) in vec2 v_uv;
#endif
