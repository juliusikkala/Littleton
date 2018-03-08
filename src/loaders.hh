#ifndef LOADERS_HH
#define LOADERS_HH
#include <string>

class resource_pool;
class scene_graph;
void load_dfo(
    resource_pool& pool,
    scene_graph& graph,
    const std::string& path,
    const std::string& data_prefix = "",
    bool ignore_duplicates = true
);
#endif
