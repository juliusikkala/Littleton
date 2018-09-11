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
#include "simple_pipeline.hh"

namespace lt
{

simple_pipeline_builder::simple_pipeline_builder(
    render_target& target,
    resource_pool& pool
): target(target), pool(pool), lighting_approach(DEFERRED)
{
}

void simple_pipeline_builder::set_algorithm(algorithm a)
{
    lighting_approach = a;
}

void simple_pipeline_builder::reset()
{
    lighting_approach = DEFERRED;
    skybox_status.enabled = false;
    atmosphere_status.enabled = false;
    bloom_status.enabled = false;
    tonemap_status.enabled = false;
    sao_status.enabled = false;
    ssao_status.enabled = false;
    ssrt_status.enabled = false;
    sg_status.enabled = false;
}

simple_pipeline* simple_pipeline_builder::build()
{
    // TODO: Do the magic here
    return nullptr;
}

}
