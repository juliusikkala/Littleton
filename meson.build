project('DarkFlowers', 'cpp', default_options : ['cpp_std=c++1z'])

src = [
  'src/main.cc',
  'src/window.cc',
  'src/resources.cc',
  'src/buffer.cc',
  'src/shader.cc',
  'src/texture.cc',
  'src/material.cc',
  'src/model.cc',
]

dfoutils_sp = subproject('DFOUtils')
dfo_dep = dfoutils_sp.get_variable('dfo_dep')

cc = meson.get_compiler('cpp')
m_dep = cc.find_library('m', required : false)
if not cc.has_header('gli/gli.hpp')
  error('Please install GLI')
endif
sdl2_dep = dependency('sdl2')
glew_dep = dependency('glew')
glm_dep = dependency('glm', version: '>=0.9.8')
thread_dep = dependency('threads')

executable(
  'dflowers',
  src,
  dependencies: [
    dfo_dep,
    m_dep,
    sdl2_dep,
    glew_dep,
    glm_dep,
    thread_dep
  ],
  install: true,
  gui_app: true,
)

