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
#include "mikktspace.h"
#include "math.hh"
#include "helpers.hh"
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

void set_image_srgb_tag(
    tinygltf::Model& model,
    tinygltf::ParameterMap& values,
    const std::string& name,
    bool value
){
    auto it = values.find(name);
    if(it != values.end())
    {
        tinygltf::Parameter& param = it->second;
        tinygltf::Texture& tex = model.textures[param.TextureIndex()];
        tinygltf::Image& image = model.images[tex.source];
        image.extras = tinygltf::Value(tinygltf::Value::Object{
            {"useSRGB", tinygltf::Value(value)}
        });
    }
}

void add_gltf_srgb_tags(tinygltf::Model& model)
{
    for(tinygltf::Material& mat: model.materials)
    {
        set_image_srgb_tag(model, mat.values, "baseColorTexture", true);
        set_image_srgb_tag(
            model,
            mat.additionalValues,
            "emissiveTexture",
            true
        );
        set_image_srgb_tag(
            model,
            mat.values,
            "metallicRoughnessTexture",
            false
        );
        set_image_srgb_tag(model, mat.additionalValues, "normalTexture", false);
    }
}

size_t read_index_from_accessor(
    tinygltf::Model& model,
    tinygltf::Accessor& index_accessor,
    size_t index
) {
    tinygltf::BufferView& view =
        model.bufferViews[index_accessor.bufferView];
    tinygltf::Buffer& buf = model.buffers[view.buffer];

    int stride = index_accessor.ByteStride(view);
    int offset = view.byteOffset + index_accessor.byteOffset;

    size_t out = 0;
    memcpy(
        &out,
        buf.data.data() + offset + stride * index,
        gl_type_sizeof(index_accessor.componentType)
    );
    return out;
}

template<typename T>
void read_from_accessor(
    tinygltf::Model& model,
    tinygltf::Accessor& accessor,
    size_t index,
    T* out
) {
    tinygltf::BufferView& view =
        model.bufferViews[accessor.bufferView];
    tinygltf::Buffer& buf = model.buffers[view.buffer];

    int stride = accessor.ByteStride(view);
    int offset = view.byteOffset + accessor.byteOffset;

    memcpy(
        out,
        buf.data.data() + offset + stride * index,
        sizeof(T)
    );
}

template<typename T>
std::vector<T> deindex(
    tinygltf::Model& model,
    tinygltf::Accessor& index_accessor,
    tinygltf::Accessor& data_accessor
) {
    std::vector<T> result(index_accessor.count);

    for(unsigned i = 0; i < index_accessor.count; ++i)
    {
        size_t index = read_index_from_accessor(model, index_accessor, i);

        read_from_accessor(
            model,
            data_accessor,
            index,
            &result[i]
        );
    }

    return result;
}

struct tangent_space_data
{
    const std::vector<vec3>& position;
    const std::vector<vec3>& normal;
    const std::vector<vec2>& texcoord;
    std::vector<vec4>& tangent;
};

int mikk_get_num_faces(const SMikkTSpaceContext* ctx)
{
    tangent_space_data* data = (tangent_space_data*)ctx->m_pUserData;
    return data->position.size()/3;
}

int mikk_get_num_vertices_of_face(const SMikkTSpaceContext*, const int)
{
    return 3;
}

void mikk_get_position(
    const SMikkTSpaceContext* ctx,
    float pos_out[],
    const int face,
    const int vert
) {
    tangent_space_data* data = (tangent_space_data*)ctx->m_pUserData;
    memcpy(
        pos_out,
        data->position.data() + face*3 + vert,
        sizeof(float)*3
    );
}

void mikk_get_normal(
    const SMikkTSpaceContext* ctx,
    float normal_out[],
    const int face,
    const int vert
) {
    tangent_space_data* data = (tangent_space_data*)ctx->m_pUserData;
    memcpy(
        normal_out,
        data->normal.data() + face*3 + vert,
        sizeof(float)*3
    );
}

void mikk_get_texcoord(
    const SMikkTSpaceContext* ctx,
    float texcoord_out[],
    const int face,
    const int vert
) {
    tangent_space_data* data = (tangent_space_data*)ctx->m_pUserData;
    memcpy(
        texcoord_out,
        data->texcoord.data() + face*3 + vert,
        sizeof(float)*2
    );
}

void mikk_set_t_space_basic(
    const SMikkTSpaceContext* ctx,
    const float tangent[],
    const float fsign,
    const int face,
    const int vert
) {
    tangent_space_data* data = (tangent_space_data*)ctx->m_pUserData;
    data->tangent[face*3 + vert] = vec4(
        tangent[0],
        tangent[1],
        tangent[2],
        fsign
    );
}

void generate_tangent_space(
    const std::vector<vec3>& position,
    const std::vector<vec3>& normal,
    const std::vector<vec2>& texcoord,
    std::vector<vec4>& tangent
) {
    tangent_space_data userdata = {
        position,
        normal,
        texcoord,
        tangent
    };

    SMikkTSpaceInterface interface = {
        mikk_get_num_faces,
        mikk_get_num_vertices_of_face,
        mikk_get_position,
        mikk_get_normal,
        mikk_get_texcoord,
        mikk_set_t_space_basic,
        nullptr
    };

    SMikkTSpaceContext ctx = {
        &interface,
        &userdata
    };

    genTangSpaceDefault(&ctx);
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
    add_gltf_srgb_tags(model);

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

        bool srgb = false;
        if(image.extras.IsObject())
        {
            tinygltf::Value::Object obj =
                image.extras.Get<tinygltf::Value::Object>();
            auto it = obj.find("useSRGB");
            if(
                it != obj.end() &&
                it->second.IsBool() &&
                it->second.Get<bool>() == true
            ) srgb = true;
        }

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
                format = srgb ? GL_SRGB8 : GL_RGB8;
                break;
            case 4:
                format = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
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
                texture::create(ctx, image.uri, srgb)
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
            gpu_buffer_accessor indices_gpu_accessor;

            std::map<primitive::attribute, gpu_buffer_accessor> attribs;

            bool use_easy_way = true;
            // Thanks for being utterly retarded bullshit glTF2!
            if(
                p.attributes.count("POSITION") &&
                p.attributes.count("NORMAL") &&
                p.attributes.count("TEXCOORD_0") &&
                !p.attributes.count("TANGENT")
            ) {
                // Missing tangent data, which must be generated using
                // MikkTSpace, which fucks up indices, so we basically have to
                // do everything from scratch.

                tinygltf::Accessor& position_accessor =
                    model.accessors[p.attributes["POSITION"]];
                tinygltf::Accessor& normal_accessor =
                    model.accessors[p.attributes["NORMAL"]];
                tinygltf::Accessor& texcoord_accessor =
                    model.accessors[p.attributes["TEXCOORD_0"]];

                // Place some common requirements to avoid overly complex
                // implementation of generate_tangent_space
                if(
                    p.mode == GL_TRIANGLES &&
                    position_accessor.componentType == GL_FLOAT &&
                    normal_accessor.componentType == GL_FLOAT &&
                    texcoord_accessor.componentType == GL_FLOAT
                ) {
                    // Do it the hard way, just because glTF2 decided fuck up
                    // our day again and sometimes not include tangent data.
                    use_easy_way = false;

                    std::vector<vec3> position(deindex<vec3>(
                        model, indices_accessor, position_accessor
                    ));

                    std::vector<vec3> normal(deindex<vec3>(
                        model, indices_accessor, normal_accessor
                    ));

                    std::vector<vec2> texcoord(deindex<vec2>(
                        model, indices_accessor, texcoord_accessor
                    ));

                    std::vector<vec4> tangent(indices_accessor.count);

                    generate_tangent_space(
                        position,
                        normal,
                        texcoord,
                        tangent
                    );

                    std::string prefix =
                        mesh.name + "[" + std::to_string(primitive_index) + "]";

                    const gpu_buffer* pos_buf = pool.add_gpu_buffer(
                        prefix + "[POSITION]",
                        gpu_buffer::create(
                            ctx, GL_ARRAY_BUFFER,
                            position.size() * sizeof(position[0]),
                            position.data()
                        )
                    );

                    const gpu_buffer* normal_buf = pool.add_gpu_buffer(
                        prefix + "[NORMAL]",
                        gpu_buffer::create(
                            ctx, GL_ARRAY_BUFFER,
                            normal.size() * sizeof(normal[0]),
                            normal.data()
                        )
                    );

                    const gpu_buffer* tangent_buf = pool.add_gpu_buffer(
                        prefix + "[TANGENT]",
                        gpu_buffer::create(
                            ctx, GL_ARRAY_BUFFER,
                            tangent.size() * sizeof(tangent[0]),
                            tangent.data()
                        )
                    );

                    const gpu_buffer* uv_buf = pool.add_gpu_buffer(
                        prefix + "[TEXCOORD_0]",
                        gpu_buffer::create(
                            ctx, GL_ARRAY_BUFFER,
                            texcoord.size() * sizeof(texcoord[0]),
                            texcoord.data()
                        )
                    );

                    // We're unindexed now.
                    attribs[primitive::POSITION] = gpu_buffer_accessor(
                        *pos_buf, 3, GL_FLOAT, false, 0, 0
                    );
                    attribs[primitive::NORMAL] = gpu_buffer_accessor(
                        *normal_buf, 3, GL_FLOAT, true, 0, 0
                    );
                    attribs[primitive::TANGENT] = gpu_buffer_accessor(
                        *tangent_buf, 4, GL_FLOAT, true, 0, 0
                    );
                    attribs[primitive::UV0] = gpu_buffer_accessor(
                        *uv_buf, 2, GL_FLOAT, false, 0, 0
                    );
                }
            }

            if(use_easy_way)
            {
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

                tinygltf::Accessor& indices_accessor = model.accessors[p.indices];

                tinygltf::BufferView& indices_view =
                    model.bufferViews[indices_accessor.bufferView];
                const gpu_buffer* indices_buf =
                    pool.get_gpu_buffer(indices_view.name);

                indices_gpu_accessor = gpu_buffer_accessor(
                    *indices_buf,
                    indices_accessor.type,
                    indices_accessor.componentType,
                    indices_accessor.normalized,
                    indices_accessor.ByteStride(indices_view),
                    indices_accessor.byteOffset
                );
            }
            primitive* prim = pool.add_primitive(
                mesh.name + "[" + std::to_string(primitive_index) + "]",
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
            primitive_index++;
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
