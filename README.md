# Littleton

Littleton is a work-in-progress game engine, licensed under LGPLv3. For now, the
engine is mostly focused on graphics, but audio, physics and networking features
are on the roadmap. This is the fifth iteration of the engine, but the first one
with a public repository. The engine is written using C++17 and targets
OpenGL 4.3 on Linux and Windows.

## Dependencies

- SDL2
- GLEW 2.0
- GLM 0.9.8
- Boost 1.62 (filesystem and system)

Additionally, these libraries are included in 'extern' (and as such, you need
not install them separately):

- stb\_image
- tiny\_gltf
- json.hpp

## Compile

### Linux

```console
$ meson build
$ ninja -C build
```

### Windows

**Note that windows build files are currently missing**

Visual Studio 2017 is used for compiling on Windows. The build files assume the
libraries are installed in:
C:\boost\_1\_67\_0
C:\SDL2-2.0.8
C:\glew-2.1.0
C:\glm-0.9.9-a2

If they aren't, you should modify both include directories and linker settings
accordingly.

## Usage

Please don't use this engine for anything yet, it's not ready for prime time and
won't be for a while. Everything can and will change and there's no
documentation. If you still want to try it out, here's what you need to do:

- Create an lt::window (this is also an lt::context instance)
- Load resources
    - Create a lt::resource\_pool
    - Load your 3D models into lt::scene\_graphs using lt::load\_gltf()
        - Merge the returned lt::scene\_graphs and add it to an lt::scene
    - Add a camera to the scene with lt::camera\_scene::set\_camera()
    - Add a few lt::lights to the scene with lt::light\_scene::add\_light()
- Create a pipeline
    - Create render target textures (lt::texture) and an lt::gbuffer (if you
      want to use deferred rendering or certain post-processing effects)
    - Create a post-processing ping-pong buffer (lt::doublebuffer)
    - Create the methods needed in the pipeline. You can find them in the
      namespace lt::method.
        - A minimal deferred shading setup requires method::clear_gbuffer,
          method::geometry_pass, method::lighting_pass and
          method::blit_framebuffer (for blitting lighting info to window)
    - Create an lt::pipeline with the methods. They will be executed on every
      frame in the order they appear in the vector.
- Call lt::pipeline::execute() for each frame
- Call lt::window::present() to show results on screen

## Features

### File support

- glTF 2.0
- JPEG, PNG, TGA, BMP, GIF, HDR, PIC, PNM

### Graphics

- PBR Materials (metallicness & roughness workflow)
- Bloom
- HDR rendering & tone mapping
- Deferred & forward rendering
- Kernel effects (3x3 convolution such as sharpen and edge detect)
- MSM and PCF shadows (directional, perspective and omnidirectional)
- Screen Space Ambient Occlusion
- Scalable Ambient Obscurance (McGuire, "Scalable Ambient Obscurance", 2012)
- Screen Space Reflections with cube map fallback
- Atmospheric scattering (Nishita, "Display of The Earth Taking into Account
  Atmospheric Scattering", 1993)
- Skyboxes
- Shader binaries
- Spherical gaussians
- Easy pipeline builder

## Wishlist

### General

- Documentation!!!
- Abstract OpenGL and SDL constants away from the user
 
### Graphics

- 2D
    - Text
    - Sprites
- Stochastic SSRT
- SSRTGI
- Bent normals
- Post-processing AA
- Bokeh DOF
- Parallax cube mapping
- Pre-convoluted cube maps
    - Specular
    - Diffuse
- Subsurface scattering
- SSRT & cube map refraction
- Cascaded shadow mapping
- Custom BRDFs
- Skeletal animation

### UI

- GUI (HTML + CSS?)
- Scene editor

### Audio

- Audio API
- Directional audio
- Convolution

### Physics

- Bullet integration
