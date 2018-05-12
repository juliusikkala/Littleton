#ifndef LOADERS_HH
#define LOADERS_HH
#include <string>
#include <unordered_map>

class resource_pool;
class scene_graph;

// GLTF files may have several scenes; return them by name.
std::unordered_map<std::string, scene_graph> load_gltf(
    resource_pool& pool,
    const std::string& path,
    const std::string& data_prefix = "",
    bool ignore_duplicates = true
);

#endif
