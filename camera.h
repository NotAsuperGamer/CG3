#pragma once

#include "geometry.h"

class Camera
{
public:
    Camera(const Vec3f& position,
           const Vec3f& target,
           const Vec3f& up_dir,
           float fov_deg,
           float aspect_ratio,
           float near_plane,
           float far_plane,
           int screen_width,
           int screen_height);

    Vec3f project(const Vec3f& vertex) const;

    [[nodiscard]] Vec3f get_position() const { return position_; }
    [[nodiscard]] Vec3f get_target() const { return target_; }
    [[nodiscard]] Vec3f get_up() const { return up_; }
    [[nodiscard]] float get_fov_deg() const { return fov_deg_; }
    [[nodiscard]] float get_aspect() const { return aspect_; }
    [[nodiscard]] float get_near_plane() const { return near_plane_; }
    [[nodiscard]] float get_far_plane() const { return far_plane_; }
    [[nodiscard]] int get_screen_width() const { return screen_width_; }
    [[nodiscard]] int get_screen_height() const { return screen_height_; }

private:
    Vec3f position_;
    Vec3f target_;
    Vec3f up_;

    float fov_deg_;
    float aspect_;
    float near_plane_;
    float far_plane_;

    int screen_width_;
    int screen_height_;
};
