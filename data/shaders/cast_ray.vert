#version 410 core

layout(location = 0) in vec2 vertex;

#ifdef CUBEMAP
layout(location = 0) out vec2 pos;
#else
layout(location = 0) out vec3 view_dir;
uniform mat4 ivp;
#ifdef LOCAL_VIEW_DIR
layout(location = 1) out vec3 local_view_dir;
uniform mat4 ip;
#endif
#endif

void main(void)
{
    gl_Position = vec4(vertex, 0.0f, 1.0f);
#ifdef CUBEMAP
    pos = vertex;
#else
    view_dir = (ivp * vec4(vertex, -1.0f, 1.0f)).xyz;
#ifdef LOCAL_VIEW_DIR
    local_view_dir = (ip * vec4(vertex, -1.0f, 1.0f)).xyz;
#endif
#endif
}

