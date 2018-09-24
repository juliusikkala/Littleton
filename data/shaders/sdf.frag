#version 410 core
#include "depth.glsl"
#include "deferred_output.glsl"

layout(location = 0) in vec3 view_dir;
layout(location = 1) in vec3 local_view_dir;

void main(void)
{
    vec3 p = normalize(local_view_dir) * 4.0f;
    gl_FragDepth = hyperbolic_depth(p.z)*0.5f+0.5f;
    write_gbuffer(
        p, -p, vec3(1),
        vec3(0), 1.0f, 0.0f, 1.0f
    );
}

