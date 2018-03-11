#include <sstream>
#include <boost/filesystem.hpp>
#include <algorithm>

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

template<typename Resource, typename Pool>
loan_returner<Resource, Pool>::loan_returner(Pool& return_target)
: return_target(return_target) { }

template<typename Resource, typename Pool>
void loan_returner<Resource, Pool>::operator()(Resource* res)
{
    return_target.give(res);
}

template<typename T>
void sorted_insert(
    std::vector<T>& vec,
    const T& value
){
    auto it = std::lower_bound(vec.begin(), vec.end(), value);
    if(it == vec.end() || *it != value) vec.insert(it, value);
}

template<typename T>
bool sorted_erase(
    std::vector<T>& vec,
    const T& value
){
    auto it = std::lower_bound(vec.begin(), vec.end(), value);
    if(it != vec.end() && *it == value)
    {
        vec.erase(it);
        return true;
    }
    return false;
}
