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
#ifndef LT_METHOD_GAMMA_HH
#define LT_METHOD_GAMMA_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"

namespace lt
{

class shader_pool;
class texture;
class resource_pool;
class sampler;

}

namespace lt::method
{

LT_OPTIONS(gamma)
{
    float gamma = 2.2f;
};

class LT_API gamma: public target_method, public options_method<gamma>
{
public:
    gamma(
        render_target& target,
        texture& src,
        resource_pool& pool,
        const options& opt = {}
    );

    void execute() override;

private:
    texture* src;

    shader* gamma_shader;

    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method

#endif

