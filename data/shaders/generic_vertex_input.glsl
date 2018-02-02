layout(location = VERTEX_POSITION) in vec3 v_vertex;
out vec3 f_position;

#ifdef VERTEX_NORMAL
layout(location = VERTEX_NORMAL) in vec3 v_normal;
out vec3 f_normal;

#ifdef VERTEX_TANGENT
layout(location = VERTEX_TANGENT) in vec4 v_tangent;
out vec3 f_tangent;
out vec3 f_bitangent;
#endif
#endif

#ifdef VERTEX_UV
layout(location = VERTEX_UV) in vec2 v_uv;
out vec2 f_uv;
#endif
