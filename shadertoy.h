#pragma once

#include <cmath>
#include <algorithm>
#include "libs/tgaimage.h"
#include "geometry.h"

struct CubeResult {
    float dist;
    Vec3f color;
    float alpha;
};

inline float sdBox(const Vec3f &p, const Vec3f &b) {
    const Vec3f q(
        std::abs(p.x) - b.x,
        std::abs(p.y) - b.y,
        std::abs(p.z) - b.z
    );
    const float max_q = std::max(std::max(q.x, q.y), q.z);
    const float min_q = std::min(std::max(q.x, std::max(q.y, q.z)), 0.0f);
    return std::sqrt(std::max(q.x, 0.0f) * std::max(q.x, 0.0f) +
                     std::max(q.y, 0.0f) * std::max(q.y, 0.0f) +
                     std::max(q.z, 0.0f) * std::max(q.z, 0.0f)) + min_q;
}

// Function to rotate a point around Y axis
inline Vec3f rotateY(const Vec3f &p, float angle) {
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    return Vec3f(
        p.x * cosA + p.z * sinA,
        p.y,
        -p.x * sinA + p.z * cosA
    );
}

// Function to rotate a point around X axis
inline Vec3f rotateX(const Vec3f &p, float angle) {
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    return Vec3f(
        p.x,
        p.y * cosA - p.z * sinA,
        p.y * sinA + p.z * cosA
    );
}

// Function to rotate a point around Z axis
inline Vec3f rotateZ(const Vec3f &p, float angle) {
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    return Vec3f(
        p.x * cosA - p.y * sinA,
        p.x * sinA + p.y * cosA,
        p.z
    );
}

inline CubeResult mapCube(const Vec3f &p) {
    const Vec3f cubePos(0.0f, 0.0f, 3.0f);
    Vec3f local_p = p - cubePos;

    // Apply rotation to make it look more 3D
    // Rotate around Y axis by 45 degrees (0.785 radians)
    local_p = rotateY(local_p, 0.785f);
    // Also rotate a bit around X axis for more perspective
    local_p = rotateX(local_p, 0.4f);
    // Add a small Z rotation for even more interesting angle
    local_p = rotateZ(local_p, 0.2f);

    // Create a hollow cube
    const float outerCube = sdBox(local_p, Vec3f(0.75f, 0.75f, 0.75f));
    const float innerCube = sdBox(local_p, Vec3f(0.615f, 0.615f, 0.615f));
    const float dist = std::max(outerCube, -innerCube); // Boolean difference

    // Transparent blue color with alpha
    Vec3f color(0.2f, 0.6f, 1.0f); // Light blue
    float alpha = 0.4f; // 40% transparency

    return CubeResult{dist, color, alpha};
}

inline float sceneSDF(const Vec3f &pos) {
    const CubeResult result = mapCube(pos);
    return result.dist;
}

inline Vec3f getCubeColor(const Vec3f &pos) {
    const CubeResult result = mapCube(pos);
    return result.color;
}

inline float getCubeAlpha(const Vec3f &pos) {
    const CubeResult result = mapCube(pos);
    return result.alpha;
}

inline Vec3f calculateNormal(const Vec3f &pos) {
    constexpr float eps = 0.001f;
    const Vec3f n(
        sceneSDF(Vec3f(pos.x + eps, pos.y, pos.z)) - sceneSDF(Vec3f(pos.x - eps, pos.y, pos.z)),
        sceneSDF(Vec3f(pos.x, pos.y + eps, pos.z)) - sceneSDF(Vec3f(pos.x, pos.y - eps, pos.z)),
        sceneSDF(Vec3f(pos.x, pos.y, pos.z + eps)) - sceneSDF(Vec3f(pos.x, pos.y, pos.z - eps))
    );
    return n.normalized();
}

inline bool raymarchSDF(const Vec3f &orig, const Vec3f &dir, float &t) {
    constexpr int max_steps = 100;
    constexpr float max_dist = 100.0f;
    constexpr float epsilon = 0.001f;

    float total_dist = 0.0f;

    for (int i = 0; i < max_steps; ++i) {
        const float dist = sceneSDF(orig + dir * total_dist);

        if (dist < epsilon) {
            t = total_dist;
            return true;
        }

        total_dist += dist;
        if (total_dist > max_dist) {
            break;
        }
    }

    return false;
}

inline void draw_shadertoy(TGAImage &picture) {
    // Don't clear the picture - it already contains the model

    // Render the transparent cube on top of the existing image
    const Vec3f light_pos(10.0f, 10.0f, -10.0f);
    const Vec3f orig(0.0f, 0.0f, 0.0f); // Camera position for cube rendering
    const int width = picture.get_width();
    const int height = picture.get_height();
    const Vec2f resolution(static_cast<float>(width), static_cast<float>(height));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Vec2f fragCoord(static_cast<float>(x), static_cast<float>(y));
            const Vec2f uv = (fragCoord - resolution * 0.5f) / resolution.y;
            Vec3f dir(uv.x, -uv.y, 1.0f);
            dir = dir.normalized();

            float t;
            if (raymarchSDF(orig, dir, t) && t > 0.0f) {
                const Vec3f hit_pos = orig + dir * t;
                const Vec3f normal = calculateNormal(hit_pos);
                const Vec3f cube_color = getCubeColor(hit_pos);
                const float alpha = getCubeAlpha(hit_pos);

                const Vec3f light_dir = (light_pos - hit_pos).normalized();
                const float intensity = std::max(normal.dot(light_dir), 0.2f);

                // Apply lighting to cube color
                Vec3f lit_color = cube_color * intensity;

                // Get existing pixel color (the model)
                TGAColor existing = picture.get(x, y);
                Vec3f existing_color(
                    existing.r / 255.0f,
                    existing.g / 255.0f,
                    existing.b / 255.0f
                );

                // Blend cube with existing color based on alpha
                Vec3f final_color = lit_color * alpha + existing_color * (1.0f - alpha);

                const unsigned char r = static_cast<unsigned char>(std::clamp(final_color.x * 255.0f, 0.0f, 255.0f));
                const unsigned char g = static_cast<unsigned char>(std::clamp(final_color.y * 255.0f, 0.0f, 255.0f));
                const unsigned char b = static_cast<unsigned char>(std::clamp(final_color.z * 255.0f, 0.0f, 255.0f));
                picture.set(x, y, TGAColor(r, g, b, 255));
            }
            // If no cube hit, keep the existing pixel (the model)
        }
    }
}