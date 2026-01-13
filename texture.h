#pragma once

#include <string>
#include <vector>

#include "geometry.h"

class Texture
{
public:
    explicit Texture(const std::string& path);
    ~Texture() = default;

    [[nodiscard]] bool is_valid() const { return !data_.empty(); }
    [[nodiscard]] Vec3f sample(const Vec2f& uv) const;

private:
    int width_;
    int height_;
    int channels_;
    std::vector<unsigned char> data_;
};
