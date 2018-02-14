#include "resources.hh"
#include <stdexcept>
#include <memory>
#include "dfo.h"
#include "vertex_buffer.hh"
#include "texture.hh"
#include "material.hh"
#include "model.hh"
#include "object.hh"
#include <glm/gtc/type_ptr.hpp>
#include <map>

resource::~resource() {}
void resource::load() const {}
void resource::unload() const {}

glresource::glresource(context& ctx): ctx(&ctx) {}
context& glresource::get_context() const { return *ctx; }

resource_store::container::~container() {};

resource_store::resource_store(context& ctx): ctx(&ctx) { }
resource_store::~resource_store() { }

class dfo_file_wrapper
{
public:
    dfo_file_wrapper(const std::string& path)
    {
        if(!dfo_open_file(&file, path.c_str(), 0))
        {
            throw std::runtime_error("Failed to open " + path);
        }
    }

    ~dfo_file_wrapper()
    {
        dfo_close(&file);
    }

    const dfo_file* get_file() const
    {
        return &file;
    }
private:
    dfo_file file;
};

class vertex_buffer_dfo: public vertex_buffer
{
public:
    vertex_buffer_dfo(
        context& ctx,
        const std::shared_ptr<dfo_file_wrapper>& res,
        dfo_buffer* buf
    ): vertex_buffer(ctx), res(res), buf(buf) {}

    void load() const override
    {
        if(vao) return;

        float* vertices = nullptr;
        uint32_t* indices = new uint32_t[buf->index_count];

        if(buf->type == DFO_VERTEX_PN)
        {
            vertices = new float[6 * buf->vertex_count];

            if(!dfo_read_buffer(res->get_file(), buf, vertices, indices))
            {
                delete [] vertices;
                delete [] indices;
                throw std::runtime_error("Failed to read DFO buffer");
            }
        }
        else if(buf->type == DFO_VERTEX_PNT)
        {
            vertices = new float[12 * buf->vertex_count];

            if(!dfo_read_buffer_tangent(
                res->get_file(),
                buf,
                12,
                vertices, vertices+3, vertices+6, vertices+10,
                indices
            )){
                delete [] vertices;
                delete [] indices;
                throw std::runtime_error("Failed to read DFO buffer");
            }
        }
        else
        {
            throw std::runtime_error(
                "Unknown DFO buffer type " + std::to_string((int)buf->type)
            );
        }

        basic_load(
            (vertex_buffer::vertex_type)buf->type,
            buf->vertex_count,
            vertices,
            buf->index_count,
            indices
        );

        delete [] vertices;
        delete [] indices;
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    std::shared_ptr<dfo_file_wrapper> res;
    dfo_buffer* buf;
};

static glm::vec4 dfo_rgba_to_vec4(dfo_rgba rgba)
{
    return glm::vec4(rgba.r, rgba.g, rgba.b, rgba.a) / 255.0f;
}

static GLint dfo_interpolation_to_gl(dfo_interpolation_type interpolation)
{
    switch(interpolation)
    {
    case DFO_INTERPOLATE_NEAREST:
        return GL_NEAREST;
    case DFO_INTERPOLATE_LINEAR:
        return GL_LINEAR_MIPMAP_LINEAR;
    default:
        throw std::runtime_error("Unknown DFO interpolation type!");
    }
}

static GLint dfo_extension_to_gl(dfo_extension_type extension)
{
    switch(extension)
    {
    case DFO_EXTENSION_REPEAT:
        return GL_REPEAT;
    case DFO_EXTENSION_CLAMP:
        return GL_CLAMP_TO_EDGE;
    case DFO_EXTENSION_CLIP:
        return GL_CLAMP_TO_BORDER;
    default:
        throw std::runtime_error("Unknown DFO extension type!");
    }
}

void resource_store::add_dfo(
    const std::string& dfo_path,
    const std::string& data_prefix,
    bool ignore_duplicates
){
    std::shared_ptr<dfo_file_wrapper> res(new dfo_file_wrapper(dfo_path));

    const dfo_file* file = res->get_file();

    // Add all vertex buffers
    std::map<dfo_buffer*, vertex_buffer*> vertex_buffers;

    for(uint32_t i = 0; i < file->buffer_count; ++i)
    {
        dfo_buffer* buf = file->buffer_table[i];
        vertex_buffers[buf] = add<vertex_buffer>(
            dfo_path + "/buffer_" + std::to_string(i),
            new vertex_buffer_dfo(*ctx, res, buf)
        );
    }

    // Add all textures
    std::map<dfo_texture*, texture*> textures;

    for(uint32_t i = 0; i < file->texture_count; ++i)
    {
        dfo_texture* tex = file->texture_table[i];
        if(ignore_duplicates && contains<texture>(tex->name)) continue;

        textures[tex] = add<texture>(
            tex->name,
            texture::create(
                *ctx,
                data_prefix.empty() ? 
                    tex->path :
                    data_prefix + "/" + tex->path,
                {
                    tex->type == DFO_TEX_SRGB_COLOR,
                    dfo_interpolation_to_gl(tex->interpolation),
                    dfo_extension_to_gl(tex->extension)
                }
            )
        );
    }

    // Add all materials
    std::map<dfo_material*, material*> materials;

    for(uint32_t i = 0; i < file->material_count; ++i)
    {
        dfo_material* mat = file->material_table[i];
        if(ignore_duplicates && contains<material>(mat->name)) continue;

        material* m = new material;

        if(mat->metallic_type == DFO_COMPONENT_CONSTANT)
            m->metallic = mat->metallic.value;
        else if(mat->metallic.tex)
            m->metallic = textures.at(mat->metallic.tex);

        if(mat->color_type == DFO_COMPONENT_CONSTANT)
            m->color = dfo_rgba_to_vec4(mat->color.value);
        else if(mat->color.tex)
            m->color = textures.at(mat->color.tex);

        if(mat->roughness_type == DFO_COMPONENT_CONSTANT)
            m->roughness = mat->roughness.value;
        else if(mat->roughness.tex)
            m->roughness = textures.at(mat->roughness.tex);

        m->ior = mat->ior;
        m->normal = textures[mat->normal];

        if(mat->emission_type == DFO_COMPONENT_CONSTANT)
            m->emission = mat->emission.value;
        else if(mat->emission.tex)
            m->emission = textures.at(mat->emission.tex);

        if(mat->subsurface_scattering_type == DFO_COMPONENT_CONSTANT)
            m->subsurface_scattering =
                dfo_rgba_to_vec4(mat->subsurface_scattering.value);
        else if(mat->subsurface_scattering.tex)
            m->subsurface_scattering =
                textures.at(mat->subsurface_scattering.tex);

        if(mat->subsurface_depth_type == DFO_COMPONENT_CONSTANT)
            m->subsurface_depth = mat->subsurface_depth.value;
        else if(mat->subsurface_depth.tex)
            m->subsurface_depth = textures.at(mat->subsurface_depth.tex);

        materials[mat] = add<material>(mat->name, m);
    }

    // Add all models.
    std::map<dfo_model*, model*> models;

    for(uint32_t i = 0; i < file->model_count; ++i)
    {
        dfo_model* mod = file->model_table[i];

        model* m = new model;
        for(uint32_t j = 0; j < mod->group_count; ++j)
        {
            dfo_vertex_group* group = mod->group_table + j;
            m->add_vertex_group(
                materials.at(group->material),
                vertex_buffers.at(group->buffer)
            );
        }

        models[mod] = add<model>(mod->name, m);
    }

    // Add objects.
    std::map<dfo_object*, object*> objects;
    for(uint32_t i = 0; i < file->object_count; ++i)
    {
        dfo_object* obj = file->object_table[i];

        object* o = new object;
        if(obj->parent) o->set_parent(objects.at(obj->parent));
        if(obj->model) o->set_model(models.at(obj->model));
        o->set_transform(glm::make_mat4(obj->transform));

        objects[obj] = add<object>(obj->name, o);
    }
}
