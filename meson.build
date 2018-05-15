# Copyright 2018 Julius Ikkala
# 
# This file is part of Littleton.
# 
# Littleton is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Littleton is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Littleton.  If not, see <http://www.gnu.org/licenses/>.

project(
  'Littleton',
  ['cpp', 'c'],
  default_options : ['cpp_std=c++1z', 'buildtype=debugoptimized']
)

src = [
  'extern/stb_image.c',
  'extern/tiny_gltf.cc',
  'src/camera.cc',
  'src/common_resources.cc',
  'src/context.cc',
  'src/doublebuffer.cc',
  'src/environment_map.cc',
  'src/framebuffer.cc',
  'src/framebuffer_pool.cc',
  'src/gbuffer.cc',
  'src/gpu_buffer.cc',
  'src/helpers.cc',
  'src/light.cc',
  'src/loaders.cc',
  'src/material.cc',
  'src/math.cc',
  'src/method/blit_framebuffer.cc',
  'src/method/bloom.cc',
  'src/method/clear.cc',
  'src/method/clear_gbuffer.cc',
  'src/method/draw_texture.cc',
  'src/method/forward_pass.cc',
  'src/method/fullscreen_effect.cc',
  'src/method/gamma.cc',
  'src/method/geometry_pass.cc',
  'src/method/kernel.cc',
  'src/method/lighting_pass.cc',
  'src/method/sao.cc',
  'src/method/shadow_method.cc',
  'src/method/shadow_msm.cc',
  'src/method/shadow_pcf.cc',
  'src/method/sky.cc',
  'src/method/skybox.cc',
  'src/method/ssao.cc',
  'src/method/ssrt.cc',
  'src/method/tonemap.cc',
  'src/method/visualize_gbuffer.cc',
  'src/model.cc',
  'src/multishader.cc',
  'src/object.cc',
  'src/pipeline.cc',
  'src/primitive.cc',
  'src/render_target.cc',
  'src/resource.cc',
  'src/resource_pool.cc',
  'src/sampler.cc',
  'src/scene.cc',
  'src/scene_graph.cc',
  'src/shader.cc',
  'src/shader_pool.cc',
  'src/shadow_map.cc',
  'src/stencil_handler.cc',
  'src/texture.cc',
  'src/transformable.cc',
  'src/uniform.cc',
  'src/window.cc',
]

cc = meson.get_compiler('cpp')
m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2')
glew_dep = dependency('glew')
glm_dep = dependency('glm', version: '>=0.9.8')
thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['filesystem', 'system'])
incdir = include_directories(['src', 'extern', 'include', 'include/method'])

liblittleton = library(
  'littleton',
  src,
  dependencies: [
    m_dep,
    sdl2_dep,
    glew_dep,
    glm_dep,
    thread_dep,
    boost_dep
  ],
  include_directories: [incdir],
  install: true,
)

littleton_dep = declare_dependency(
  include_directories : incdir,
  link_with : liblittleton
)
