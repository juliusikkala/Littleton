#include <functional>
#include <sstream>
#include <boost/filesystem.hpp>

template<typename T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T, typename Hash>
std::string append_hash_to_path(
    const std::string& prefix,
    const T& hashable,
    const std::string& suffix
){
    Hash hasher;

    std::stringstream ss;
    ss << std::hex << hasher(hashable);

    boost::filesystem::path hash_path(ss.str() + suffix);
    boost::filesystem::path prefix_path(prefix);
    return (prefix_path/hash_path).string();
}
