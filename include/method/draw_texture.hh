#ifndef LT_METHOD_DRAW_TEXTURE_HH
#define LT_METHOD_DRAW_TEXTURE_HH
#include "pipeline.hh"
#include "primitive.hh"
#include "sampler.hh"

namespace lt
{

class texture;
class resource_pool;

}

namespace lt::method
{

class draw_texture: public target_method
{
public:
    draw_texture(
        render_target& target,
        resource_pool& shaders,
        texture* tex = nullptr
    );
    ~draw_texture();

    void set_transform(glm::mat4 transform);

    void set_texture(texture* tex = nullptr);

    void execute() override;

    std::string get_name() const override;

private:
    const primitive& quad;
    sampler color_sampler;

    shader* draw_shader;
    glm::mat4 transform;
    texture* tex;
};

} // namespace lt::method

#endif

