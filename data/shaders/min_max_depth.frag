#version 400 core

uniform sampler2D prev;
uniform bool handle_both_edges;
uniform bool handle_top_edge;
uniform bool handle_right_edge;
in vec2 uv;

#if !defined(MAXIMUM) || !defined(MINIMUM)
out float result;
#else
out vec2 result;
#endif

vec2 mip_gather()
{
    ivec2 p = ivec2(gl_FragCoord.xy) * 2 + 1;

    if(handle_both_edges)
    {
        vec2 s[] = vec2[](
            texelFetchOffset(prev, p, 0, ivec2(-1,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(1,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(1,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(1,1)).xy
        );
        vec2 res = s[0];
        for(int i = 1; i < 9; ++i)
            res = vec2(min(res.x, s[i].x), max(res.y, s[i].y));
        return res;
    }
    else if(handle_right_edge)
    {
        vec2 s[] = vec2[](
            texelFetchOffset(prev, p, 0, ivec2(-1,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(1,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(1,0)).xy
        );
        vec2 res = s[0];
        for(int i = 1; i < 6; ++i)
            res = vec2(min(res.x, s[i].x), max(res.y, s[i].y));
        return res;
    }
    else if(handle_top_edge)
    {
        vec2 s[] = vec2[](
            texelFetchOffset(prev, p, 0, ivec2(-1,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,1)).xy
        );
        vec2 res = s[0];
        for(int i = 1; i < 6; ++i)
            res = vec2(min(res.x, s[i].x), max(res.y, s[i].y));
        return res;
    }
    else
    {
        vec2 s[] = vec2[](
            texelFetchOffset(prev, p, 0, ivec2(0,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(0,-1)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,0)).xy,
            texelFetchOffset(prev, p, 0, ivec2(-1,-1)).xy
        );

        return vec2(
            min(min(s[0].x, s[1].x), min(s[2].x, s[3].x)),
            max(max(s[0].y, s[1].y), max(s[2].y, s[3].y))
        );
    }
}

void main(void)
{
#if defined(MAXIMUM) && defined(MINIMUM)
    result = mip_gather();
#elif defined(MAXIMUM)
    result = mip_gather().y;
#elif defined(MINIMUM)
    result = mip_gather().x;
#endif
}
