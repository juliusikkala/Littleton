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
#ifndef LT_METHOD_SHADOW_MSM_HH
#define LT_METHOD_SHADOW_MSM_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../render_target.hh"
#include "../texture.hh"
#include "../framebuffer.hh"
#include "../sampler.hh"
#include "shadow_method.hh"

namespace lt
{

class resource_pool;
class shader;
class primitive;
namespace method { class shadow_msm; }

}


namespace lt
{

class LT_API directional_shadow_map_msm: public directional_shadow_map
{
friend class method::shadow_msm;
public:
    directional_shadow_map_msm(
        method::shadow_msm* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        unsigned radius = 4,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );

    directional_shadow_map_msm(directional_shadow_map_msm&& other);

    unsigned get_samples() const;

    void set_radius(unsigned radius);
    unsigned get_radius() const;

    texture& get_moments();
    const texture& get_moments() const;

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

class LT_API omni_shadow_map_msm: public omni_shadow_map
{
friend class method::shadow_msm;
public:
    omni_shadow_map_msm(
        method::shadow_msm* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 4,
        // Currently unused due to difficulties in cube map blurring
        float radius = 1,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );
    omni_shadow_map_msm(omni_shadow_map_msm&& other);

    unsigned get_samples() const;

    texture& get_moments();
    const texture& get_moments() const;

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
};

class LT_API perspective_shadow_map_msm: public perspective_shadow_map
{
friend class method::shadow_msm;
public:
    perspective_shadow_map_msm(
        method::shadow_msm* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        unsigned radius = 4,
        double fov = 30,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );

    perspective_shadow_map_msm(perspective_shadow_map_msm&& other);

    unsigned get_samples() const;

    void set_radius(unsigned radius);
    unsigned get_radius() const;

    texture& get_moments();
    const texture& get_moments() const;

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

} // namespace lt

namespace lt::method
{

class LT_API shadow_msm: public glresource, public shadow_method
{
public:
    shadow_msm(resource_pool& pool, render_scene* scene);

    shader::definition_map get_directional_definitions() const override;
    shader::definition_map get_omni_definitions() const override;
    shader::definition_map get_perspective_definitions() const override;

    void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        directional_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) override;

    void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        omni_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) override;

    void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        perspective_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) override;

    void execute() override;

    std::string get_name() const override;

private:
    resource_pool& pool;

    shader* depth_shader;
    shader* cubemap_depth_shader;
    shader* perspective_depth_shader;

    shader* vertical_blur_shader;
    shader* horizontal_blur_shader;

    const primitive& quad;
    sampler moment_sampler;
    sampler cubemap_moment_sampler;
};

} // namespace lt::method

#endif
