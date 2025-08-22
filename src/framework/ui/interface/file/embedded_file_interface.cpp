#include "embedded_file_interface.hpp"

#include "framework/resource/resource.hpp"

LOAD_RESOURCE(resources_textures_mart_png)

Rml::FileHandle embedded_file_interface::Open(const Rml::String& path) {
    std::stringstream* s;

    if (path == "mart.png")
        s = new std::stringstream(
            std::string(
                GET_RESOURCE(resources_textures_mart_png).begin(), GET_RESOURCE(resources_textures_mart_png).end()
            )
        );
    else
        s = nullptr;

    return std::bit_cast<Rml::FileHandle>(s);
}

void embedded_file_interface::Close(Rml::FileHandle file) {
    auto s = std::bit_cast<std::stringstream*>(file);
    delete s;
}

size_t embedded_file_interface::Read(void* buffer, size_t size, Rml::FileHandle file) {
    auto s = std::bit_cast<std::stringstream*>(file);
    return s->readsome(std::bit_cast<char*>(buffer), size);
}

bool embedded_file_interface::Seek(Rml::FileHandle file, long offset, int origin) {
    auto s = std::bit_cast<std::stringstream*>(file);
    s->seekg(offset, origin);
    return s->good();
}

size_t embedded_file_interface::Tell(Rml::FileHandle file) {
    auto s = std::bit_cast<std::stringstream*>(file);
    return s->tellg();
}