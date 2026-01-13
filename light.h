#pragma once

#include "geometry.h"

class Light
{
public:
    explicit Light(const Vec3f& direction);
    Light(const Vec3f& direction, const Vec3f& color, float intensity);

    [[nodiscard]] Vec3f get_direction() const { return direction_; }
    [[nodiscard]] Vec3f get_color() const { return color_; }
    [[nodiscard]] float get_intensity() const { return intensity_; }

private:
    Vec3f direction_;
    Vec3f color_;
    float intensity_;
};
