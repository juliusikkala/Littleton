project(
  'DarkFlowers',
  'cpp',
  default_options : ['cpp_std=c++1z', 'buildtype=debugoptimized']
)

src = [
  'lib/stb_image.c',
  'src/main.cc',
  'src/window.cc',
  'src/resources.cc',
  'src/vertex_buffer.cc',
  'src/shader.cc',
  'src/texture.cc',
  'src/material.cc',
  'src/model.cc',
  'src/object.cc',
  'src/pipeline.cc',
  'src/scene.cc',
  'src/camera.cc',
  'src/transformable.cc',
  'src/helpers.cc',
  'src/multishader.cc',
  'src/uniform.cc',
  'src/light.cc',
  'src/render_target.cc',
  'src/framebuffer.cc',
  'src/context.cc',
  'src/gbuffer.cc',
  'src/shader_store.cc',
  'src/method/clear.cc',
  'src/method/fullscreen_effect.cc',
  'src/method/forward_pass.cc',
  'src/method/geometry_pass.cc',
  'src/method/blit_framebuffer.cc',
  'src/method/lighting_pass.cc',
]

dfoutils_sp = subproject('DFOUtils', default_options : ['buildtype=release'])
dfo_dep = dfoutils_sp.get_variable('dfo_dep')

cc = meson.get_compiler('cpp')
m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2')
glew_dep = dependency('glew')
glm_dep = dependency('glm', version: '>=0.9.8')
thread_dep = dependency('threads')
boost_dep = dependency('boost', modules : ['filesystem', 'system'])
srcdir = include_directories('src')
libdir = include_directories('lib')

executable(
  'dflowers',
  src,
  dependencies: [
    dfo_dep,
    m_dep,
    sdl2_dep,
    glew_dep,
    glm_dep,
    thread_dep,
    boost_dep
  ],
  include_directories: [srcdir, libdir],
  install: true,
  gui_app: true,
)

