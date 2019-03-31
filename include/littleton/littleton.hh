/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_LITTLETON_HH
#define LT_LITTLETON_HH

#include "about.hh"
#include "animated.hh"
#include "camera.hh"
#include "common_resources.hh"
#include "context.hh"
#include "doublebuffer.hh"
#include "environment_map.hh"
#include "font.hh"
#include "framebuffer.hh"
#include "framebuffer_pool.hh"
#include "gbuffer.hh"
#include "glheaders.hh"
#include "gpu_buffer.hh"
#include "light.hh"
#include "loaders.hh"
#include "loaner.hh"
#include "material.hh"
#include "math.hh"
#include "model.hh"
#include "multishader.hh"
#include "object.hh"
#include "pipeline.hh"
#include "primitive.hh"
#include "render_target.hh"
#include "resource.hh"
#include "resource_pool.hh"
#include "sampler.hh"
#include "scene.hh"
#include "scene_graph.hh"
#include "sdf.hh"
#include "shader.hh"
#include "shader_pool.hh"
#include "shadow_map.hh"
#include "simple_pipeline.hh"
#include "spherical_gaussians.hh"
#include "sprite.hh"
#include "stencil_handler.hh"
#include "texture.hh"
#include "timer.hh"
#include "transformable.hh"
#include "uniform.hh"
#include "window.hh"

#include "method/apply_sg.hh"
#include "method/atmosphere.hh"
#include "method/blit_framebuffer.hh"
#include "method/bloom.hh"
#include "method/clear.hh"
#include "method/clear_gbuffer.hh"
#include "method/draw_texture.hh"
#include "method/forward_pass.hh"
#include "method/fullscreen_effect.hh"
#include "method/gamma.hh"
#include "method/geometry_pass.hh"
#include "method/generate_depth_mipmap.hh"
#include "method/generate_sg.hh"
#include "method/kernel.hh"
#include "method/lighting_pass.hh"
#include "method/sao.hh"
#include "method/sdf.hh"
#include "method/shadow_method.hh"
#include "method/shadow_msm.hh"
#include "method/shadow_pcf.hh"
#include "method/skybox.hh"
#include "method/ssao.hh"
#include "method/ssrt.hh"
#include "method/tonemap.hh"
#include "method/visualize_cubemap.hh"
#include "method/visualize_gbuffer.hh"

#endif
