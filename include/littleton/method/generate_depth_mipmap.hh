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
#ifndef LT_METHOD_GENERATE_DEPTH_MIPMAP_HH
#define LT_METHOD_GENERATE_DEPTH_MIPMAP_HH
#include "../api.hh"
#include "../pipeline.hh"

namespace lt
{

class gbuffer;
class sampler;
class shader;
class primitive;
class resource_pool;

}

namespace lt::method
{

class LT_API generate_depth_mipmap: public target_method
{
public:
    generate_depth_mipmap(gbuffer& buf, resource_pool& pool);

    void execute() override;

private:
    gbuffer* buf;
    shader* min_max_shader;
    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method

#endif

