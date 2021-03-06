/*
    Copyright 2019 Julius Ikkala

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
#include "sprite.hh"
#include "common_resources.hh"

namespace lt
{

sprite_layout::sprite_layout()
:   sprite_layout(
        std::vector<mode>{{{}, {{{}, {{vec4(0,0,1,1), vec2(0.5, 0.5)}}}}}}
    )
{}

sprite_layout::sprite_layout(const std::vector<mode>& layout)
: layout(layout)
{
    for(const mode& m: layout)
    {
        duration loop_length;
        for(const mode::frame& f: m.animation_frames)
            loop_length += f.frame_time;
        animation_length.push_back(loop_length);
    }
}

sprite_layout::tile sprite_layout::get_tile(
    unsigned index,
    duration animation_time,
    vec3 view,
    bool looping,
    bool* directional_cap
) const
{
    if(index >= layout.size()) return tile{vec4(0), vec2(0)};

    const mode& m = layout[index];
    if(m.animation_frames.size() == 0) return tile{vec4(0), vec2(0)};

    // Resolve animation frame, if relevant
    unsigned frame_index = 0;
    if(m.animation_frames.size() > 1)
    {
        duration time;

        if(looping)
            animation_time = animation_time % animation_length[index];

        for(;
            frame_index < m.animation_frames.size() &&
            time <= animation_time;
            ++frame_index
        ) time += m.animation_frames[frame_index].frame_time;
        frame_index--;
    }

    const mode::frame& f = m.animation_frames[frame_index];
    if(f.tiles.size() == 0) return tile{vec4(0), vec2(0)};

    // Directional sprite handling (find nearest provided direction).
    // Non-directional sprites do not enter the if below and return f.tiles[0].
    unsigned closest_index = 0;

    if(m.directions.size() >= 1)
    {
        float closest_distance = dot(view, m.directions[0].view);
        for(unsigned i = 1; i < m.directions.size(); ++i)
        {
            float dist = dot(view, m.directions[i].view);
            if(dist > closest_distance)
            {
                closest_distance = dist;
                closest_index = i;
            }
        }
    }

    if(directional_cap)
        *directional_cap = m.directions[closest_index].cap;

    return f.tiles[closest_index];
}

sprite_layout sprite_layout::simple_v(unsigned modes, vec2 origin)
{
    std::vector<mode> layout;
    vec2 coord(0, 0);
    vec2 size(1.0, 1.0/modes);
    for(unsigned i = 0; i < modes; ++i)
    {
        coord.y = i/(float)modes;
        layout.push_back({{}, {{{}, {{vec4(coord, coord+size), origin}}}}});
    }
    return sprite_layout(layout);
}

sprite_layout sprite_layout::simple_h(unsigned modes, vec2 origin)
{
    std::vector<mode> layout;
    vec2 coord(0, 0);
    vec2 size(1.0/modes, 1.0);
    for(unsigned i = 0; i < modes; ++i)
    {
        coord.x = i/(float)modes;
        layout.push_back({{}, {{{}, {{vec4(coord, coord+size), origin}}}}});
    }
    return sprite_layout(layout);
}

sprite_layout sprite_layout::animated_v(
    unsigned frames,
    duration frame_time,
    unsigned modes,
    vec2 origin
){
    std::vector<mode> layout;
    vec2 coord(0, 0);
    vec2 size(1.0/frames, 1.0/modes);
    for(unsigned i = 0; i < modes; ++i)
    {
        coord.y = i/(float)modes;
        mode m;
        for(unsigned j = 0; j < frames; ++j)
        {
            coord.x = j/(float)frames;
            m.animation_frames.push_back(
                {frame_time, {{vec4(coord, coord+size), origin}}}
            );
        }
        
        layout.push_back(m);
    }
    return sprite_layout(layout);
}

sprite_layout sprite_layout::animated_h(
    unsigned frames,
    duration frame_time,
    unsigned modes,
    vec2 origin
){
    std::vector<mode> layout;
    vec2 coord(0, 0);
    vec2 size(1.0/modes, 1.0/frames);
    for(unsigned i = 0; i < modes; ++i)
    {
        coord.x = i/(float)modes;
        mode m;
        for(unsigned j = 0; j < frames; ++j)
        {
            coord.y = j/(float)frames;
            m.animation_frames.push_back(
                {frame_time, {{vec4(coord, coord+size), origin}}}
            );
        }
        
        layout.push_back(m);
    }
    return sprite_layout(layout);
}

sprite_layout sprite_layout::directional(
    unsigned yaw_steps, unsigned pitch_steps, vec2 origin
){
    if(pitch_steps <= 1)
        return directional(yaw_steps, std::vector<float>{0.0f}, origin);
 
    float pitch_step_size = 180.0 / pitch_steps;
    float start = -90.0 + pitch_step_size * 0.5;
    std::vector<float> pitch_steps_vec(pitch_steps);
    for(unsigned i = 0; i < pitch_steps; ++i)
        pitch_steps_vec[i] = start + i * pitch_step_size;
    return directional(yaw_steps, pitch_steps_vec, origin);
}

sprite_layout sprite_layout::directional(
    unsigned yaw_steps, const std::vector<float>& pitch_steps, vec2 origin
){
    if(yaw_steps <= 1)
        return directional(std::vector<float>{0.0f}, pitch_steps, origin);

    float yaw_step_size = 360.0 / yaw_steps;
    std::vector<float> yaw_steps_vec(yaw_steps);
    for(unsigned i = 0; i < yaw_steps; ++i)
        yaw_steps_vec[i] = i * yaw_step_size;
    return directional(yaw_steps_vec, pitch_steps, origin);
}

sprite_layout sprite_layout::directional(
    const std::vector<float>& yaw_steps,
    const std::vector<float>& pitch_steps,
    vec2 origin
){
    vec2 coord(0, 0);
    vec2 size(1.0/yaw_steps.size(), 1.0/pitch_steps.size());
    std::vector<tile> tiles;
    std::vector<mode::direction_info> directions;
    for(unsigned i = 0; i < pitch_steps.size(); ++i)
    {
        coord.y = i / (float)pitch_steps.size();
        for(unsigned j = 0; j < yaw_steps.size(); ++j)
        {
            coord.x = j / (float)yaw_steps.size();
            tiles.push_back({vec4(coord, coord+size), origin});
            directions.push_back(
                {pitch_yaw_to_vec(pitch_steps[i], yaw_steps[j]), false}
            );
        }
    }
    return sprite_layout({{directions, {{{}, tiles}}}});
}

sprite::sprite(
    resource_pool& pool,
    const std::string& texture_path,
    const sprite_layout* layout,
    interpolation mag,
    interpolation min
):  mode(0), animation_looping(true), default_mag(mag), default_min(min),
    origin(0), layout(layout)
{
    set_texture(pool, texture_path);
}

sprite::sprite(
    const texture* tex,
    const sprite_layout* layout,
    interpolation mag,
    interpolation min
):  mode(0), animation_looping(true), default_mag(mag), default_min(min),
    origin(0), layout(layout)
{
    set_texture(tex);
}

sprite::sprite(
    const material& mat,
    const sprite_layout* layout,
    interpolation mag,
    interpolation min
):  mode(0), animation_looping(true), default_mag(mag), default_min(min),
    mat(mat), origin(0), layout(layout)
{}

sprite::sprite(const sprite_sheet& sheet, unsigned mode)
:   mode(mode), animation_looping(sheet.animation_looping),
    default_mag(sheet.default_mag), default_min(sheet.default_min),
    mat(sheet.mat), origin(0), layout(&sheet.layout)
{
}

void sprite::set_mode(unsigned mode)
{
    this->mode = mode;
}

unsigned sprite::get_mode() const
{
    return mode;
}

const sprite_layout* sprite::get_layout() const
{
    return layout;
}

const texture* sprite::get_texture() const
{
    return mat.color_texture.second;
}

void sprite::set_texture(const texture* tex)
{
    mat.color_texture.second = tex;
}

void sprite::set_texture(const texture* tex, const sprite_layout* layout)
{
    mat.color_texture.second = tex;
    this->layout = layout;
}

void sprite::set_texture(resource_pool& pool, const std::string& texture_path)
{
    if(pool.contains_texture(texture_path))
        set_texture(pool.get_texture(texture_path));
    else set_texture(pool.add_texture(
        texture_path,
        texture::create(pool.get_context(), texture_path)
    ));
}

void sprite::set_texture(
    resource_pool& pool,
    const std::string& texture_path,
    const sprite_layout* layout
){
    set_texture(pool, texture_path);
    this->layout = layout;
}

const material& sprite::get_material() const
{
    return mat;
}

void sprite::set_material(const material& mat)
{
    this->mat = mat;
}

void sprite::set_material(const material& mat, const sprite_layout* layout)
{
    this->mat = mat;
    this->layout = layout;
}

void sprite::set_color(vec4 color_factor)
{
    mat.color_factor = color_factor;
}

vec4 sprite::get_color() const
{
    return mat.color_factor;
}

void sprite::set_interpolation(interpolation both)
{
    set_interpolation(both, both);
}

void sprite::set_interpolation(interpolation mag, interpolation min)
{
    default_mag = mag;
    default_min = min;
}

void sprite::get_interpolation(interpolation& mag, interpolation& min) const
{
    mag = default_mag;
    min = default_min;
}

void sprite::set_animation_looping(bool looping)
{
    this->animation_looping = looping;
}

bool sprite::get_animation_looping() const
{
    return animation_looping;
}

void sprite::set_origin(vec2 origin)
{
    this->origin = origin;
}

vec2 sprite::get_origin() const
{
    return origin;
}

sprite_layout::tile sprite::get_tile(
    vec3 view,
    bool& directional_cap
) const
{
    if(layout == nullptr)
        return sprite_layout::tile{vec4(0,0,1,1), origin};

    sprite_layout::tile tile = layout->get_tile(
        mode,
        get_animation_time(),
        view,
        animation_looping,
        &directional_cap
    );
    tile.origin += origin;
    return tile;
}

sprite_sheet::sprite_sheet(
    const texture* tex,
    const sprite_layout& layout,
    interpolation mag,
    interpolation min
): animation_looping(true), default_mag(mag), default_min(min), layout(layout)
{
    mat.color_texture.second = tex;
}

sprite_sheet::sprite_sheet(
    const material& mat,
    const sprite_layout& layout,
    interpolation mag,
    interpolation min
):  animation_looping(true), default_mag(mag), default_min(min), mat(mat),
    layout(layout)
{
}

sprite_sheet::sprite_sheet(
    resource_pool& pool,
    const std::string& texture_path,
    const sprite_layout& layout,
    interpolation mag,
    interpolation min
): animation_looping(true), default_mag(mag), default_min(min), layout(layout)
{
    if(pool.contains_texture(texture_path))
        set_texture(pool.get_texture(texture_path));
    else set_texture(pool.add_texture(
        texture_path,
        texture::create(pool.get_context(), texture_path)
    ));
}

const texture* sprite_sheet::get_texture() const
{
    return mat.color_texture.second;
}

void sprite_sheet::set_texture(const texture* tex)
{
    mat.color_texture.second = tex;
}

void sprite_sheet::set_interpolation(interpolation both)
{
    set_interpolation(both, both);
}

void sprite_sheet::set_interpolation(interpolation mag, interpolation min)
{
    default_mag = mag;
    default_min = min;
}

void sprite_sheet::set_animation_looping(bool looping)
{
    animation_looping = looping;
}

bool sprite_sheet::get_animation_looping() const
{
    return animation_looping;
}

const material& sprite_sheet::get_material() const
{
    return mat;
}

material& sprite_sheet::get_material()
{
    return mat;
}

const sprite_layout& sprite_sheet::get_layout() const
{
    return layout;
}

sprite_layout& sprite_sheet::get_layout()
{
    return layout;
}

sprite sprite_sheet::create_sprite(unsigned mode) const
{
    return sprite(*this, mode);
}

} // namespace lt

