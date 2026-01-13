#include "camera.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
    constexpr float kEpsilon = 1e-6f;
}

Camera::Camera(const Vec3f& position,
               const Vec3f& target,
               const Vec3f& up_dir,
               float fov_deg,
               float aspect_ratio,
               float near_plane,
               float far_plane,
               int screen_width,
               int screen_height)
    : position_(position),
      target_(target),
      up_(up_dir),
      fov_deg_(fov_deg),
      aspect_(aspect_ratio),
      near_plane_(near_plane),
      far_plane_(far_plane),
      screen_width_(screen_width),
      screen_height_(screen_height)
{
}

Vec3f Camera::project(const Vec3f& vertex) const
{
    const Vec3f z = (position_ - target_).normalized();
    const Vec3f x = up_.cross(z).normalized();
    const Vec3f y = z.cross(x);

    const Vec3f translated = vertex - position_;
    const float vx = translated.dot(x);
    const float vy = translated.dot(y);
    float vz = translated.dot(z);

    if (std::abs(vz) < kEpsilon)
    {
        vz = (vz >= 0.0f) ? kEpsilon : -kEpsilon;
    }

    const float fov_rad = fov_deg_ * std::numbers::pi_v<float> / 180.0f;
    const float f = 1.0f / std::tan(fov_rad / 2.0f);
    const float px = (vx / -vz) * f * aspect_;
    const float py = (vy / -vz) * f;
    const float normalized_depth = std::clamp((-(vz) - near_plane_) / (far_plane_ - near_plane_), 0.0f, 1.0f);

    const float sx = (px + 1.0f) * 0.5f * static_cast<float>(screen_width_);
    const float sy = (py + 1.0f) * 0.5f * static_cast<float>(screen_height_);
    const float sz = 1.0f - normalized_depth;

    return {sx, sy, sz};
}
