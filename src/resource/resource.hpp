#pragma once
#include "common.hpp"

struct resource {
    resource(const char* begin, const char* end) : _begin(begin), _end(end) {}
    [[nodiscard]] const char* data() const { return _begin; }
    [[nodiscard]] const char* begin() const { return _begin; }
    [[nodiscard]] const char* end() const { return _end; }
    [[nodiscard]] size_t size() const { return static_cast<size_t>(_end - _begin); }
    [[nodiscard]] std::string_view str() const { return std::string_view{this->data(), this->size()}; }
    [[nodiscard]] std::span<const std::byte> bytes() const {
        return {
            reinterpret_cast<const std::byte*>(this->begin()),
            reinterpret_cast<const std::byte*>(this->end())
    };
    }
private:
    const char* _begin;
    const char* _end;
};

#define LOAD_RESOURCE(x) extern "C" char _binary_##x##_start, _binary_##x##_end;
#define GET_RESOURCE(x) resource{&_binary_##x##_start, &_binary_##x##_end}