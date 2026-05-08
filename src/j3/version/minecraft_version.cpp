#include "minecraft_version.hpp"

minecraft_version::minecraft_version(
    const uint16_t major, const uint16_t minor, const uint16_t build, const uint16_t revision
) : major(major), minor(minor), build(build), revision(revision) { }

minecraft_version::minecraft_version(const std::string& version) {
    constexpr auto delim = std::string_view{ "." };
    
    int i = 0;
    for (const auto number : std::views::split(version, delim)) {
        switch (i) {
        case 0:
            std::from_chars(number.data(), number.data() + number.size(), this->major);
            break;
        case 1:
            std::from_chars(number.data(), number.data() + number.size(), this->minor);
            break;
        case 2:
            std::from_chars(number.data(), number.data() + number.size(), this->build);
            break;
        case 3:
            std::from_chars(number.data(), number.data() + number.size(), this->revision);
            break;
            
        default:
            spdlog::warn("Version string too large");
            break;
        }
        
        i++;
    }
}

std::string minecraft_version::string(const bool friendly) const {
    if (friendly) {
        return {
            this->is_gdk()
            ? std::format("{}.{}", this->minor, this->build)
            : std::format("1.{}.{}", this->minor, this->build)
        };
    }
    
    return std::format("{}.{}.{}.{}", this->major, this->minor, this->build, this->revision);
}

bool minecraft_version::empty() const {
    return std::tuple{ this->major, this->minor, this->build, this->revision } == std::tuple{ 0, 0, 0, 0 };
}

bool minecraft_version::is_gdk() const {
    return std::tuple{ this->major, this->minor, this->patch } >= std::tuple{ 1, 21, 120 };
}
