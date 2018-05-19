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
#ifndef LT_METHOD_KERNEL_HH
#define LT_METHOD_KERNEL_HH
#include "../pipeline.hh"
#include "../primitive.hh"

namespace lt
{

class texture;
class resource_pool;
class sampler;

}

namespace lt::method
{

class kernel: public target_method
{
public:
    static const glm::mat3 SHARPEN;
    static const glm::mat3 EDGE_DETECT;
    static const glm::mat3 GAUSSIAN_BLUR;
    static const glm::mat3 BOX_BLUR;

    kernel(
        render_target& target,
        texture& src,
        resource_pool& store,
        const glm::mat3& k = SHARPEN
    );

    void set_kernel(const glm::mat3& kernel);
    glm::mat3 get_kernel() const;

    void execute() override;

    std::string get_name() const override;

private:
    texture* src;
    shader* kernel_shader;
    const primitive& quad;
    const sampler& fb_sampler;
    glm::mat3 k;
};

} // namespace lt::method
#endif
