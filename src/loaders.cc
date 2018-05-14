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
#include "loaders.hh"
#include "resource_pool.hh"
#include "scene_graph.hh"
#include "primitive.hh"
#include "texture.hh"
#include "material.hh"
#include "model.hh"
#include "object.hh"
#include "tiny_gltf.h"
#include "math.hh"
#include <stdexcept>
#include <memory>
#include <map>

namespace
{
using namespace lt;

void load_gltf_node(
    resource_pool& pool,
    tinygltf::Model& model,
    tinygltf::Scene& scene,
    tinygltf::Node& node,
    scene_graph& graph,
    bool ignore_duplicates,
    transformable_node* parent = nullptr
){
    // TODO: Add camera support to scene graph
    if(node.mesh == -1) return;

    object obj;
    obj.set_parent(parent);

    // Set transformation for object
    if(node.matrix.size())
        obj.set_transform(glm::make_mat4(node.matrix.data()));
    else
    {
        if(node.translation.size())
            obj.set_position(glm::make_vec3(node.translation.data()));

        if(node.scale.size())
            obj.set_scaling(glm::make_vec3(node.scale.data()));

        if(node.rotation.size())
            obj.set_orientation(glm::make_quat(node.rotation.data()));
    }

    tinygltf::Mesh& mesh = model.meshes[node.mesh];
    obj.set_model(pool.get_model(mesh.name));

    // Add object to graph
    object* obj_ref = graph.add_object(
        node.name,
        std::move(obj),
        ignore_duplicates
    );

    // Load child nodes
    for(int node_index: node.children)
    {
        load_gltf_node(
            pool,
            model,
            scene,
            model.nodes[node_index],
            graph,
            ignore_duplicates,
            obj_ref
        );
    }
}

material::sampler_tex get_material_texture_parameter(
    resource_pool& pool,
    tinygltf::Model& model,
    tinygltf::ParameterMap& values,
    const std::string& name,
    float* scale = nullptr
){
    auto it = values.find(name);
    if(it != values.end())
    {
        tinygltf::Parameter& param = it->second;
        tinygltf::Texture& tex = model.textures[param.TextureIndex()];
        tinygltf::Sampler& sampler = model.samplers[tex.sampler];
        tinygltf::Image& image = model.images[tex.source];
        if(scale)
        {
            auto it = param.json_double_value.find("scale");
            if(it != param.json_double_value.end())
                *scale = it->second;
            else
                *scale = 1.0f;
        }
        return material::sampler_tex(
            pool.get_sampler(sampler.name),
            pool.get_texture(image.name)
        );
    }
    if(scale) *scale = 0;
    return material::sampler_tex(nullptr, nullptr);
}

glm::vec4 get_material_factor_parameter(
    tinygltf::ParameterMap& values,
    const std::string& name,
    glm::vec4 fallback = glm::vec4(1)
){
    auto it = values.find(name);
    if(it != values.end())
    {
        tinygltf::Parameter& param = it->second;
        if(param.has_number_value) return glm::vec4(param.number_value);
        else
        {
            return glm::vec4(
                param.number_array[0],
                param.number_array[1],
                param.number_array[2],
                param.number_array.size() == 4 ? param.number_array[3] : 1.0f
            );
        }
    }
    return fallback;
}

template<typename T>
void ensure_gltf_uniquely_named(
    std::vector<T>& array,
    const std::string& prefix
){
    unsigned index = 0;
    for(T& item: array)
    {
        if(item.name == "")
            item.name = prefix + "[" + std::to_string(index++) + "]";
    }
}

}

namespace lt
{

std::unordered_map<std::string, scene_graph> load_gltf(
    resource_pool& pool,
    const std::string& path,
    const std::string& data_prefix,
    bool ignore_duplicates
){
    std::unordered_map<std::string, scene_graph> scenes;
    context& ctx = pool.get_context();

    std::string err;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    if(!loader.LoadBinaryFromFile(&model, &err, path))
    {
        throw std::runtime_error(err);
    }

    ensure_gltf_uniquely_named(model.accessors, path+"/unnamed_accessor");
    ensure_gltf_uniquely_named(model.animations, path+"/unnamed_animation");
    ensure_gltf_uniquely_named(model.buffers, path+"/unnamed_buffer");
    ensure_gltf_uniquely_named(model.bufferViews, path+"/unnamed_buffer_view");
    ensure_gltf_uniquely_named(model.materials, path+"/unnamed_material");
    ensure_gltf_uniquely_named(model.meshes, path+"/unnamed_mesh");
    ensure_gltf_uniquely_named(model.nodes, path+"/unnamed_node");
    ensure_gltf_uniquely_named(model.images, path+"/unnamed_image");
    ensure_gltf_uniquely_named(model.skins, path+"/unnamed_skin");
    ensure_gltf_uniquely_named(model.samplers, path+"/unnamed_sampler");
    ensure_gltf_uniquely_named(model.cameras, path+"/unnamed_camera");
    ensure_gltf_uniquely_named(model.scenes, path+"/unnamed_scene");
    ensure_gltf_uniquely_named(model.lights, path+"/unnamed_light");

    // Add samplers
    for(tinygltf::Sampler& s: model.samplers)
    {
        if(ignore_duplicates && pool.contains_sampler(s.name))
            continue;

        pool.add_sampler(
            s.name,
            new sampler(ctx, s.magFilter, s.minFilter, s.wrapS)
        );
    }

    // Add textures
    for(tinygltf::Image& image: model.images)
    {
        if(ignore_duplicates && pool.contains_texture(image.name))
            continue;

        if(image.bufferView != -1)
        {// Embedded image
            GLint format;

            switch(image.component)
            {
            case 1:
                format = GL_R8;
                break;
            case 2:
                format = GL_RG8;
                break;
            default:
            case 3:
                format = GL_SRGB8;
                break;
            case 4:
                format = GL_SRGB8_ALPHA8;
                break;
            }

            pool.add_texture(
                image.name,
                texture::create(
                    ctx,
                    glm::uvec2(image.width, image.height),
                    format,
                    GL_UNSIGNED_BYTE,
                    0,
                    GL_TEXTURE_2D,
                    image.image.size(),
                    image.image.data()
                )
            );
        }
        else
        {// URI
            pool.add_texture(
                image.name,
                texture::create(ctx, image.uri, true)
            );
        }
    }

    // Add materials
    for(tinygltf::Material& mat: model.materials)
    {
        if(ignore_duplicates && pool.contains_texture(mat.name))
            continue;

        material* m = new material;

        m->color_factor = get_material_factor_parameter(
            mat.values, "baseColorFactor"
        );
        m->color_texture = get_material_texture_parameter(
            pool, model, mat.values, "baseColorTexture"
        );

        m->metallic_factor = get_material_factor_parameter(
            mat.values, "metallicFactor"
        ).x;

        m->roughness_factor = get_material_factor_parameter(
            mat.values, "roughnessFactor"
        ).x;

        m->metallic_roughness_texture = get_material_texture_parameter(
            pool, model, mat.values, "metallicRoughnessTexture"
        );

        m->normal_texture = get_material_texture_parameter(
            pool, model, mat.additionalValues, "normalTexture",
            &m->normal_factor
        );

        m->ior = 1.45f;

        m->emission_factor = get_material_factor_parameter(
            mat.additionalValues, "emissiveFactor", glm::vec4(0)
        );

        m->emission_texture = get_material_texture_parameter(
            pool, model, mat.additionalValues, "emissiveTexture"
        );

        pool.add_material(mat.name, m);
    }

    // Load buffers
    for(tinygltf::BufferView& view: model.bufferViews)
    {
        tinygltf::Buffer& buf = model.buffers[view.buffer];
        if(view.target == 0)
        {
            // For some reason, the Blender glTF2 plugin exports a targetless
            // bufferView when using .glb (not when using .gltf). As far as I
            // know, it seems to be completely unused...
            continue;
        }
        pool.add_gpu_buffer(
            view.name,
            gpu_buffer::create(
                ctx,
                view.target,
                view.byteLength,
                buf.data.data() + view.byteOffset
            )
        );
    }

    // Load models
    for(tinygltf::Mesh& mesh: model.meshes)
    {
        lt::model* m = new lt::model();

        unsigned primitive_index = 0;
        for(tinygltf::Primitive& p: mesh.primitives)
        {
            tinygltf::Accessor& indices_accessor = model.accessors[p.indices];

            tinygltf::BufferView& indices_view =
                model.bufferViews[indices_accessor.bufferView];
            const gpu_buffer* indices_buf =
                pool.get_gpu_buffer(indices_view.name);

            gpu_buffer_accessor indices_gpu_accessor(
                *indices_buf,
                indices_accessor.type,
                indices_accessor.componentType,
                indices_accessor.normalized,
                indices_accessor.ByteStride(indices_view),
                indices_accessor.byteOffset
            );

            std::map<primitive::attribute, gpu_buffer_accessor> attribs;
            const std::map<
                std::string,
                primitive::attribute
            > known_attributes = {
                {"POSITION", primitive::POSITION},
                {"NORMAL", primitive::NORMAL},
                {"TANGENT", primitive::TANGENT},
                {"TEXCOORD_0", primitive::UV0},
                {"TEXCOORD_1", primitive::UV1},
                {"TEXCOORD_2", primitive::UV2},
                {"TEXCOORD_3", primitive::UV3}
            };

            unsigned user_index = primitive::USER_INDEX;

            for(const auto& pair: p.attributes)
            {
                tinygltf::Accessor& accessor = model.accessors[pair.second];
                tinygltf::BufferView& view =
                    model.bufferViews[accessor.bufferView];
                const gpu_buffer* buf = pool.get_gpu_buffer(view.name);
                
                primitive::attribute attrib;
                auto it = known_attributes.find(pair.first);
                if(it == known_attributes.end())
                    attrib = primitive::attribute{
                        user_index++, "VERTEX_" + pair.first
                    };
                else attrib = it->second;

                attribs[attrib] = gpu_buffer_accessor(
                    *buf,
                    accessor.type,
                    accessor.componentType,
                    accessor.normalized,
                    accessor.ByteStride(view),
                    accessor.byteOffset
                );
            }

            primitive* prim = pool.add_primitive(
                mesh.name + "[" + std::to_string(primitive_index++) + "]",
                primitive::create(
                    ctx,
                    indices_accessor.count,
                    p.mode,
                    indices_gpu_accessor,
                    attribs
                )
            );

            m->add_vertex_group(
                p.material < 0 ?
                    nullptr :
                    pool.get_material(model.materials[p.material].name),
                prim
            );
        }

        pool.add_model(mesh.name, m);
    }

    // Add objects
    for(tinygltf::Scene& scene: model.scenes)
    {
        scene_graph& graph = scenes[scene.name];

        for(int node_index: scene.nodes)
        {
            load_gltf_node(
                pool,
                model,
                scene,
                model.nodes[node_index],
                graph,
                ignore_duplicates
            );
        }
    }

    return scenes;
}

} // namespace lt
