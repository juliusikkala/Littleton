namespace lt
{

template<typename T>
std::vector<T> texture::read() const
{
    load();
    size_t texels = dimensions.x * dimensions.y * dimensions.z;
    size_t data_bytes = texels * get_texel_size();
    std::vector<T> res((data_bytes + sizeof(T) - 1) / sizeof(T));
    glBindTexture(target, tex);
    glGetTexImage(
        target,
        0,
        get_external_format(),
        type,
        res.data()
    );
    return res;
}

}
