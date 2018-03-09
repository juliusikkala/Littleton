#ifndef FRAMEBUFFER_POOL_HH
#define FRAMEBUFFER_POOL_HH
#include "resource.hh"
#include "framebuffer.hh"
#include "helpers.hh"
#include <unordered_map>
#include <memory>

class framebuffer_pool: public virtual glresource
{
public:
    framebuffer_pool(context& ctx);
    framebuffer_pool(const framebuffer_pool& other) = delete;
    framebuffer_pool(framebuffer_pool& other) = delete;
    ~framebuffer_pool();

    framebuffer* take(
        glm::uvec2 size,
        const framebuffer::target_specification_map& target_specifications,
        unsigned samples = 0
    );

    void give(framebuffer* fb);

    using loaner = ::loaner<framebuffer, framebuffer_pool>;

    loaner loan(
        glm::uvec2 size,
        const framebuffer::target_specification_map& target_specifications,
        unsigned samples = 0
    );

    void unload_all();

    struct key
    {
        framebuffer::target_specification_map target_specifications;
        unsigned samples;

        bool operator==(const key& other) const;
    };

    struct key_hash{ size_t operator()(const key& k) const; };

    using map_type = std::unordered_map<
        key,
        std::set<framebuffer*>,
        key_hash
    >;

private:
    void confirm_specifications(
        const framebuffer::target_specification_map& target_specifications
    ) const;

    map_type framebuffers;
};

#endif
