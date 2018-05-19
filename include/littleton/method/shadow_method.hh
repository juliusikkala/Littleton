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
#ifndef LT_METHOD_SHADOW_METHOD_HH
#define LT_METHOD_SHADOW_METHOD_HH
#include "../pipeline.hh"
#include "../render_target.hh"
#include "../shadow_map.hh"

namespace lt
{

class directional_light;
class render_scene;

}

namespace lt::method
{

class shadow_method: public pipeline_method
{
public:
    shadow_method(render_scene* scene = nullptr);

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    // Sets the uniforms needed when using directional shadow maps with
    // this method.
    virtual void set_directional_uniforms(
        shader* s,
        unsigned& texture_index
    );

    // Sets the uniforms needed when using omnidirectional shadow maps with
    // this method.
    virtual void set_omni_uniforms(shader* s, unsigned& texture_index);

    // Sets the uniforms needed when using perspective shadow maps with
    // this method.
    virtual void set_perspective_uniforms(
        shader* s,
        unsigned& texture_index
    );

    // Definitions needed when using the shadow maps.
    virtual shader::definition_map get_directional_definitions() const;
    virtual shader::definition_map get_omni_definitions() const;
    virtual shader::definition_map get_perspective_definitions() const;

    // Sets shadow map uniforms. The given shadow map is assumed to be a  type
    // compatible with the method.
    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        directional_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        omni_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        perspective_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

protected:
    render_scene* scene;
};

} // namespace lt::method

#endif
