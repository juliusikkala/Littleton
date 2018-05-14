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
#include "framebuffer_pool.hh"
#include <boost/functional/hash.hpp>

namespace lt
{

framebuffer_pool::framebuffer_pool(context& ctx)
: glresource(ctx)
{
}

framebuffer_pool::~framebuffer_pool()
{
    unload_all();
}

framebuffer* framebuffer_pool::take(
    glm::uvec2 size,
    const framebuffer::target_specification_map& target_specifications,
    unsigned samples
){
    confirm_specifications(target_specifications);

    key k{target_specifications, samples};

    auto map_it = framebuffers.find(k);
    if(map_it == framebuffers.end())
        map_it = framebuffers.emplace(k, std::set<framebuffer*>{}).first;

    std::set<framebuffer*>& compatible_specs = map_it->second;

    for(auto it = compatible_specs.begin(); it != compatible_specs.end(); ++it)
    {
        framebuffer* fb = *it;
        if(fb->get_size() == size)
        {
            compatible_specs.erase(it);
            return fb;
        }
    }

    framebuffer* fb = 
        new framebuffer(get_context(), size, target_specifications, samples);
    compatible_specs.insert(fb);
    return fb;
}

void framebuffer_pool::give(framebuffer* fb)
{
    if(!fb) return;

    key k{fb->get_target_specifications(), fb->get_samples()};

    auto it = framebuffers.find(k);
    if(it == framebuffers.end())
        it = framebuffers.emplace(k, std::set<framebuffer*>{}).first;

    it->second.insert(fb);
}

framebuffer_pool::loaner framebuffer_pool::loan(
    glm::uvec2 size,
    const framebuffer::target_specification_map& target_specifications,
    unsigned samples
){
    confirm_specifications(target_specifications);
    return loaner(
        take(size, target_specifications, samples),
        loan_returner<framebuffer, framebuffer_pool>(*this)
    );
}

void framebuffer_pool::unload_all()
{
    for(auto& pair: framebuffers)
    {
        for(framebuffer* fb: pair.second)
        {
            delete fb;
        }
    }
    framebuffers.clear();
}

bool framebuffer_pool::key::operator==(const key& other) const
{
    return other.samples == samples &&
        other.target_specifications == target_specifications;
}

size_t framebuffer_pool::key_hash::operator()(const key& k) const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, k.target_specifications);
    boost::hash_combine(seed, k.samples);
    return seed;
}

void framebuffer_pool::confirm_specifications(
    const framebuffer::target_specification_map& target_specifications
) const{
    for(auto& pair: target_specifications)
    {
        if(pair.second.use_texture != nullptr)
            throw std::runtime_error(
                "target_specifier::use_texture must be nullptr "
                "for framebuffer_pool"
            );
    }
}

} // namespace lt
