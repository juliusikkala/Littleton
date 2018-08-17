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
#ifndef LT_METHOD_TONEMAP_HH
#define LT_METHOD_TONEMAP_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"

namespace lt
{

class texture;
class resource_pool;

}

namespace lt::method
{

LT_OPTIONS(tonemap)
{
    float exposure = 1.0f;
};

class LT_API tonemap: public target_method, public options_method<tonemap>
{
public:
    tonemap(
        render_target& target,
        resource_pool& pool,
        texture* src,
        const options& opt = {}
    );

    void execute() override;

    std::string get_name() const override;

private:
    texture* src;
    shader* tonemap_shader;
    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method
#endif
