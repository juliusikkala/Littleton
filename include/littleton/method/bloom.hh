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
#ifndef LT_METHOD_BLOOM_HH
#define LT_METHOD_BLOOM_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"

namespace lt
{

class texture;
class resource_pool;
class multishader;

}

namespace lt::method
{

LT_OPTIONS(bloom)
{
    float threshold = 2.0f;
    unsigned radius = 5;
    float strength = 1.0f;
    unsigned level = 2;
};

class LT_API bloom: public target_method, public options_method<bloom>
{
public:
    bloom(
        render_target& target,
        resource_pool& pool,
        texture* src,
        const options& opt = {}
    );

    void execute() override;

    std::string get_name() const override;

protected:
    void options_will_update(const options& next);

private:
    resource_pool& pool;

    texture* src;

    shader* threshold_shader;
    multishader* convolution_shader;
    shader* apply_shader;

    std::vector<float> gaussian_kernel;

    const primitive& quad;
    sampler smooth_sampler;
};
}
#endif
