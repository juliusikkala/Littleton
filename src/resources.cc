#include "resources.hh"
#include <stdexcept>
#include <memory>
#include "dfo.h"
#include "buffer.hh"
#include "texture.hh"
#include "material.hh"
#include "model.hh"
#include "object.hh"
#include <glm/gtc/type_ptr.hpp>
#include <map>

basic_resource_ptr::~basic_resource_ptr()
{
    reset(nullptr);
}

void basic_resource_ptr::pin() const
{
    if(s)
    {
        local_pins++;
        s->global_pins++;
        if(s->global_pins == 1)
        {
            s->resource = s->create_resource();
        }
    }
}

void basic_resource_ptr::unpin() const
{
    if(s && local_pins != 0)
    {
        local_pins--;
        s->global_pins--;
        if(s->global_pins == 0)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }
    }
}

bool basic_resource_ptr::operator==(const basic_resource_ptr& other) const
{
    return s && other.s == s;
}

bool basic_resource_ptr::operator!=(const basic_resource_ptr& other) const
{
    return !s || other.s != s;
}

basic_resource_ptr::operator bool() const
{
    return s;
}

basic_resource_ptr::basic_resource_ptr()
: local_pins(0), s(nullptr) { }

basic_resource_ptr::basic_resource_ptr(shared* other_s)
: local_pins(0), s(other_s) { if(s) s->references++; }

basic_resource_ptr::basic_resource_ptr(const basic_resource_ptr& other)
: local_pins(0), s(other.s) { if(s) s->references++; }

basic_resource_ptr::basic_resource_ptr(basic_resource_ptr&& other)
: local_pins(other.local_pins), s(other.s)
{
    other.local_pins = 0;
    other.s = nullptr;
}

void basic_resource_ptr::reset(shared* other_s)
{
    if(s)
    {
        s->global_pins -= local_pins;
        if(s->global_pins == 0 && s->resource)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }

        s->references--;
        if(s->references == 0)
        {
            if(s->resource)
            {
                s->delete_resource(s->resource);
            }
            delete s;
        }
    }

    local_pins = 0;
    s = other_s;
    if(s) s->references++;
}

basic_resource_ptr& basic_resource_ptr::operator=(basic_resource_ptr&& other)
{
    if(s)
    {
        s->global_pins -= local_pins;
        if(s->global_pins == 0 && s->resource)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }

        s->references--;
        if(s->references == 0)
        {
            if(s->resource)
            {
                s->delete_resource(s->resource);
            }
            delete s;
        }
    }
    s = other.s;
    local_pins = other.local_pins;

    other.local_pins = 0;
    other.s = nullptr;
    return *this;
}

basic_resource_ptr& basic_resource_ptr::operator=(
    const basic_resource_ptr& other
){
    reset(other.s);
    return *this;
}

resource_store::resource_store() { }

resource_store::~resource_store() { }

class dfo_file_resource
{
public:
    dfo_file_resource(const std::string& path)
    {
        if(!dfo_open_file(&file, path.c_str(), 0))
        {
            throw std::runtime_error("Failed to open " + path);
        }
    }

    ~dfo_file_resource()
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

using dfo_ptr = resource_ptr<dfo_file_resource>;

class dfo_buffer_reader
{
public:
    dfo_buffer_reader(dfo_ptr res, dfo_buffer* buf)
    : res(res), buf(buf)
    {
        this->res.pin();
    }

    dfo_buffer_reader(const dfo_buffer_reader& other)
    : res(other.res), buf(other.buf)
    {
        this->res.pin();
    }

    ~dfo_buffer_reader()
    {
    }

    void* operator()()
    {
        buffer* new_buf;
        if(buf->present)
        {
            new_buf = new buffer(
                (buffer::buffer_type)buf->type,
                buf->data.data,
                buf->size
            );
        }
        else
        {
            char* data = new char[buf->size];
            if(!dfo_read_buffer(res->get_file(), buf, data))
            {
                delete [] data;
                throw std::runtime_error("Failed to read DFO buffer");
            }
            new_buf = new buffer(
                (buffer::buffer_type)buf->type,
                data,
                buf->size
            );
            delete [] data;
        }
        return new_buf;
    }

private:
    dfo_ptr res;
    dfo_buffer* buf;
};

static color4 dfo_rgba_to_color4(dfo_rgba rgba)
{
    return color4(rgba.r, rgba.g, rgba.b, rgba.a) / 255.0f;
}

void resource_store::add_dfo(const std::string& dfo_path)
{
    dfo_ptr res = create<dfo_file_resource>(dfo_path, dfo_path);

    const dfo_file* file = res->get_file();

    // Add all buffers
    std::map<dfo_buffer*, buffer_ptr> buffers;

    for(uint32_t i = 0; i < file->buffer_count; ++i)
    {
        dfo_buffer* buf = file->buffer_table[i];
        buffers[buf] = buffer_ptr(dfo_buffer_reader(res, buf));
    }

    // Add all textures
    std::map<dfo_texture*, texture_ptr> textures;

    for(uint32_t i = 0; i < file->texture_count; ++i)
    {
        dfo_texture* tex = file->texture_table[i];
        textures[tex] = create<texture>(tex->path, std::string(tex->path));
    }

    // Add all materials
    std::map<dfo_material*, material_ptr> materials;

    for(uint32_t i = 0; i < file->material_count; ++i)
    {
        dfo_material* mat = file->material_table[i];
        material* m = new material;

        if(mat->metallic_type == DFO_COMPONENT_CONSTANT)
            m->metallic = mat->metallic.value;
        else if(mat->metallic.tex)
            m->metallic = textures.at(mat->metallic.tex);

        if(mat->color_type == DFO_COMPONENT_CONSTANT)
            m->color = dfo_rgba_to_color4(mat->color.value);
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
                dfo_rgba_to_color4(mat->subsurface_scattering.value);
        else if(mat->subsurface_scattering.tex)
            m->subsurface_scattering =
                textures.at(mat->subsurface_scattering.tex);

        if(mat->subsurface_depth_type == DFO_COMPONENT_CONSTANT)
            m->subsurface_depth = mat->subsurface_depth.value;
        else if(mat->subsurface_depth.tex)
            m->subsurface_depth = textures.at(mat->subsurface_depth.tex);

        materials[mat] = add(mat->name, material_ptr(m));
    }

    // Add all models.
    std::map<dfo_model*, model_ptr> models;

    for(uint32_t i = 0; i < file->model_count; ++i)
    {
        dfo_model* mod = file->model_table[i];
        model* m = new model;
        for(uint32_t j = 0; j < mod->group_count; ++j)
        {
            dfo_vertex_group* group = mod->group_table + j;
            m->add_vertex_group(
                materials.at(group->material),
                buffers.at(group->vertex_buffer),
                buffers.at(group->index_buffer)
            );
        }

        models[mod] = add(mod->name, model_ptr(m));
    }

    // Add objects.
    std::map<dfo_object*, object_ptr> objects;
    for(uint32_t i = 0; i < file->object_count; ++i)
    {
        dfo_object* obj = file->object_table[i];
        object* o = new object;
        if(obj->parent) o->set_parent(objects.at(obj->parent));
        if(obj->model) o->set_model(models.at(obj->model));
        o->set_transform(glm::make_mat4(obj->transform));
        objects[obj] = add(obj->name, object_ptr(o));
    }
}

void resource_store::pin_all()
{
    for(auto& pair: resources)
    {
        pair.second.pin();
    }
}

void resource_store::unpin_all()
{
    for(auto& pair: resources)
    {
        pair.second.unpin();
    }
}

bool resource_store::name_type::operator==(const name_type& other) const
{
    return other.type == type && other.name == name;
}

size_t resource_store::name_type_hash::operator()(const name_type& nt) const
{
    return nt.type.hash_code() + std::hash<std::string>()(nt.name);
}

