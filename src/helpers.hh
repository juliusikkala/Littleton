#ifndef HELPERS_HH
#define HELPERS_HH
#include <string>
#include <map>

std::string read_text_file(const std::string& path);

template<typename T>
void hash_combine(std::size_t& seed, const T& v);

template<
    class K,
    class T,
    class C = std::less<K>,
    class A = std::allocator<std::pair<const K, T>>
>
struct map_hasher
{
    size_t operator()(const std::map<K, T, C, A>& arg) const noexcept
    {
        size_t hash = arg.size();
        for(auto& pair: arg)
        {
            hash_combine(hash, pair.first);
            hash_combine(hash, pair.second);
        }
        return hash;
    }
};

#include "helpers.tcc"

#endif
