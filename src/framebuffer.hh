#ifndef FRAMEBUFFER_HH
#define FRAMEBUFFER_HH
#include "render_target.hh"
#include <vector>
#include <map>
#include <memory>
#include <variant>

class texture;
class framebuffer: public render_target
{
public:
    struct target_specifier
    {
        target_specifier(
            GLint format = GL_RGBA,
            bool as_texture = false
        );

        target_specifier(texture* use_texture);

        GLint format;
        bool as_texture;
        texture* use_texture;

        bool operator==(const target_specifier& other) const;
    };

    using target_specification_map = std::map<GLenum, target_specifier>;

    framebuffer(
        context& ctx,
        glm::uvec2 size,
        const target_specification_map& target_specifications = {},
        unsigned samples = 0
    );

    framebuffer(framebuffer&& f);
    ~framebuffer();

    const target_specification_map& get_target_specifications() const;
    unsigned get_samples() const;

    texture* get_texture_target(GLenum attachment) const;

private:
    target_specification_map target_specifications;
    unsigned samples;

    std::vector<std::unique_ptr<texture>> owned_textures;

    std::map<GLenum, std::variant<texture*, GLuint>> targets;
};

namespace boost
{
    size_t hash_value(const framebuffer::target_specifier& t);
}

#endif
