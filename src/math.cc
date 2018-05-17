/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "math.hh"

namespace
{
using namespace lt;

vec2 circle_projection_range(vec2 dir, float r, float p, float big)
{
    float d2 = glm::dot(dir, dir);
    float r2 = r * r;

    if(d2 <= r2) { return glm::vec2(-1, 1) * big; }

    float len = sqrt(d2 - r2);
    vec2 n = dir / dir.y;

    vec2 h(-n.y, n.x);
    h *= r / len;

    vec2 up = n + h;
    float top = up.x / fabs(up.y) * p;

    vec2 down = n - h;
    float bottom = down.x / fabs(down.y) * p;

    if(dir.x > 0 && dir.y <= r)
    {
        bottom = big;
        if(dir.y <= 0) top = -top;
    }

    if(dir.x < 0 && dir.y <= r)
    {
        top = -big;
        if(dir.y <= 0) bottom = -bottom;
    }

    return vec2(top, bottom);
}

}

namespace lt
{

void decompose_matrix(
    const glm::mat4& transform,
    glm::vec3& translation,
    glm::vec3& scaling,
    glm::quat& orientation
){
    translation = transform[3];
    scaling = glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );
    orientation = glm::quat(glm::mat4(
        transform[0]/scaling.x,
        transform[1]/scaling.y,
        transform[2]/scaling.z,
        glm::vec4(0,0,0,1)
    ));
}

glm::vec3 get_matrix_translation(const glm::mat4& transform)
{
    return transform[3];
}

glm::vec3 get_matrix_scaling(const glm::mat4& transform)
{
    return glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );
}

glm::quat get_matrix_orientation(const glm::mat4& transform)
{
    return glm::quat(glm::mat4(
        glm::normalize(transform[0]),
        glm::normalize(transform[1]),
        glm::normalize(transform[2]),
        glm::vec4(0,0,0,1)
    ));
}

void decompose_perspective(
    const glm::mat4& perspective,
    float& near,
    float& far,
    float& fov,
    float& aspect
){
    float a = perspective[2][2];
    float b = perspective[3][2];
    float c = perspective[0][0];
    float d = perspective[1][1];

    near = fabs(-b/(1-a));
    far = fabs(b/(1+a));
    fov = 2*atan(1/d);
    aspect = d/c;
}

glm::quat rotate_towards(
    glm::quat orig,
    glm::quat dest,
    float angle_limit
){
    angle_limit = glm::radians(angle_limit);

    float cos_theta = dot(orig, dest);
    if(cos_theta > 0.999999f)
    {
        return dest;
    }

    if(cos_theta < 0)
    {
        orig *= -1.0f;
        cos_theta *= -1.0f;
    }

    float theta = acos(cos_theta);
    if(theta < angle_limit) return dest;
    return glm::mix(orig, dest, angle_limit/theta);
}

glm::quat quat_lookat(
    glm::vec3 dir,
    glm::vec3 up,
    glm::vec3 forward
){
    dir = glm::normalize(dir);
    up = glm::normalize(up);
    forward = glm::normalize(forward);

    glm::quat towards = glm::rotation(
        glm::vec3(0,0,-1),
        forward
    );
    return towards * glm::quatLookAt(dir, up);
}

bool solve_quadratic(float a, float b, float c, float& x0, float& x1)
{
    float D = b * b - 4 * a * c;
    float sD = sqrt(D) * sign(a);
    float denom = -0.5f/a;
    x0 = (b + sD) * denom;
    x1 = (b - sD) * denom;
    return !std::isnan(sD);
}

bool intersect_sphere(
    glm::vec3 pos,
    glm::vec3 dir,
    glm::vec3 origin,
    float radius,
    float& t0,
    float& t1
){
    glm::vec3 L = pos - origin;
    float a = glm::dot(dir, dir);
    float b = 2*glm::dot(dir, L);
    float c = glm::dot(L, L) - radius * radius;

    if(!solve_quadratic(a, b, c, t0, t1)) return false;
    if(t1 < 0) return false;
    if(t0 < 0) t0 = 0;

    return true;
}

unsigned next_power_of_two(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

unsigned factorize(unsigned n)
{
    // Divisible by two
    if((n&1)==0) return 2;

    unsigned last = floor(sqrt(n));
    for(unsigned i = 3; i <= last; ++i)
        if((n % i) == 0) return i;

    return 0;
}

glm::mat4 sphere_projection_quad_matrix(
    glm::vec3 pos,
    float r,
    float near,
    float far,
    bool use_near_radius,
    float big
){

    float d = -pos.z;

    if(use_near_radius) d = glm::max(d - r, near);
    else d = glm::min(d + r, far);

    glm::vec2 w = circle_projection_range(
        glm::vec2(pos.x, -pos.z),
        r, d, big
    );
    glm::vec2 h = circle_projection_range(
        glm::vec2(pos.y, -pos.z),
        r, d, big
    );

    glm::vec2 center = glm::vec2(w.x + w.y, h.x + h.y) / 2.0f;
    glm::vec2 scale = glm::vec2(fabs(w.y - w.x), fabs(h.y - h.x)) / 2.0f;

    return glm::translate(glm::vec3(center, -d)) *
           glm::scale(glm::vec3(scale, 0));
}

template<typename T, class F>
void mitchell_best_candidate(
    std::vector<T>& samples,
    F&& sample_generator,
    unsigned candidate_count,
    unsigned count
){
    if(count < samples.size()) return;

    samples.reserve(count);
    count -= samples.size();

    while(count--)
    {
        T farthest = T(0);
        float farthest_distance = 0;

        for(unsigned i = 0; i < candidate_count; ++i)
        {
            T candidate = sample_generator();
            float candidate_distance = INFINITY;

            for(const T& sample: samples)
            {
                float dist = glm::distance(candidate, sample);
                if(dist < candidate_distance) candidate_distance = dist;
            }

            if(candidate_distance > farthest_distance)
            {
                farthest_distance = candidate_distance;
                farthest = candidate;
            }
        }

        samples.push_back(farthest);
    }
}

void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [r](){return glm::diskRand(r);},
        candidate_count,
        count
    );
}

void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float w,
    float h,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [w, h](){
            return glm::linearRand(glm::vec2(-w/2, -h/2), glm::vec2(w/2, h/2));
        },
        candidate_count,
        count
    );
}

void mitchell_best_candidate(
    std::vector<glm::vec3>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
){
    mitchell_best_candidate(
        samples,
        [r](){return glm::ballRand(r);},
        candidate_count,
        count
    );
}

std::vector<glm::vec2> grid_samples(
    unsigned w,
    unsigned h,
    float step
){
    std::vector<glm::vec2> samples;
    samples.resize(w*h);

    glm::vec2 start(
        (w-1)/-2.0f,
        (h-1)/-2.0f
    );

    for(unsigned i = 0; i < h; ++i)
        for(unsigned j = 0; j < w; ++j)
            samples[i*w+j] = start + glm::vec2(i, j) * step;

    return samples;
}

std::vector<float> generate_gaussian_kernel(
    int radius,
    float sigma
){
    std::vector<float> result;
    result.reserve(radius * 2 + 1);

    for(int i = -radius; i <= radius; ++i)
    {
        float f = i/sigma;
        float weight = exp(-f*f/2.0f)/(sigma * sqrt(2*M_PI));
        result.push_back(weight);
    }
    return result;
}

unsigned calculate_mipmap_count(uvec2 size)
{
    return floor(log2(std::max(size.x, size.y)))+1;
}

} // namespace lt
