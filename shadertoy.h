#pragma once

#include <cmath>
#include <algorithm>
#include "libs/tgaimage.h"
#include "geometry.h"

struct Ship {
    Vec3f position;
    float angle;
    bool isCentral;
};

struct ShipResult {
    float dist;
    Vec3f color;
};

inline float sdPlane(const Vec3f &p, const Vec3f &n, float h) {
    return p.dot(n.normalized()) + h;
}

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

inline float sdCylinder(const Vec3f &p, float r, float h) {
    const float d_xz = std::sqrt(p.x * p.x + p.z * p.z);
    const Vec2f d(std::abs(d_xz - r), std::abs(p.y) - h);
    const float max_d = std::max(d.x, d.y);
    const float min_d = std::min(max_d, 0.0f);
    return std::sqrt(std::max(d.x, 0.0f) * std::max(d.x, 0.0f) +
                     std::max(d.y, 0.0f) * std::max(d.y, 0.0f)) + min_d;
}

inline float sdCylinderZ(const Vec3f &p, float r, float h) {
    const float d_xy = std::sqrt(p.x * p.x + p.y * p.y);
    const Vec2f d(std::abs(d_xy - r), std::abs(p.z) - h);
    const float max_d = std::max(d.x, d.y);
    const float min_d = std::min(max_d, 0.0f);
    return std::sqrt(std::max(d.x, 0.0f) * std::max(d.x, 0.0f) +
                     std::max(d.y, 0.0f) * std::max(d.y, 0.0f)) + min_d;
}

inline float sdUnion(float d1, float d2) {
    return std::min(d1, d2);
}

inline Vec2f rotY(float angle) {
    return Vec2f(std::cos(angle), std::sin(angle));
}

inline float mapHull(const Vec3f &p, const Vec3f &shipPos) {
    Vec3f local_p = p - shipPos;

    const float body = sdBox(local_p, Vec3f(1.6f, 0.2f, 0.6f));
    const float bridge = sdBox(local_p - Vec3f(0.0f, 0.3f, 0.0f), Vec3f(0.4f, 0.2f, 0.3f));

    return sdUnion(body, bridge);
}

inline float mapTurret(const Vec3f &p, float angle, const Vec3f &shipPos, bool isCentral) {
    Vec3f q = p - shipPos - Vec3f(0.0f, 0.5f, 0.0f);

    const float turretAngle = isCentral ? angle : 0.0f;
    const Vec2f rot = rotY(turretAngle + 1.5708f); // +90 degrees in radians
    const float new_x = q.x * rot.x - q.z * rot.y;
    const float new_z = q.x * rot.y + q.z * rot.x;
    q.x = new_x;
    q.z = new_z;

    const float base = sdCylinder(q, 0.23f, 0.3f);
    const float head = sdBox(q - Vec3f(0.0f, 0.14f, 0.0f), Vec3f(0.24f, 0.1f, 0.28f));
    const Vec3f barrel = q - Vec3f(0.0f, 0.15f, 0.35f);
    const float gun = sdCylinderZ(barrel, 0.05f, 0.3f);
    const float gunThingy = sdCylinderZ(barrel, 0.07f, 0.14f);
    const float gun_combined = std::min(gun, gunThingy);

    return std::min(base, std::min(head, gun_combined));
}

inline ShipResult mapShipSingle(const Vec3f &p, float angle, const Vec3f &shipPos, bool isCentral) {
    const float hull = mapHull(p, shipPos);
    const float turret = mapTurret(p, angle, shipPos, isCentral);
    const float dist = sdUnion(hull, turret);

    Vec3f col(0.4f, 0.45f, 0.5f);
    if (std::abs(dist - hull) < 0.001f) {
        col = isCentral ? Vec3f(0.1f, 0.2f, 0.4f) : Vec3f(0.2f, 0.1f, 0.3f);
    } else if (std::abs(dist - turret) < 0.001f) {
        col = isCentral ? Vec3f(0.3f, 0.35f, 0.4f) : Vec3f(0.4f, 0.3f, 0.35f);
    }

    return ShipResult{dist, col};
}

inline float sceneSDF(const Vec3f &pos, const Ship &ship) {
    const ShipResult result = mapShipSingle(pos, ship.angle, ship.position, ship.isCentral);
    return result.dist;
}

inline Vec3f getShipColor(const Vec3f &pos, const Ship &ship) {
    const ShipResult result = mapShipSingle(pos, ship.angle, ship.position, ship.isCentral);
    return result.color;
}

inline Vec3f calculateNormal(const Vec3f &pos, const Ship &ship) {
    constexpr float eps = 0.001f;
    const Vec3f n(
            sceneSDF(Vec3f(pos.x + eps, pos.y, pos.z), ship) - sceneSDF(Vec3f(pos.x - eps, pos.y, pos.z), ship),
            sceneSDF(Vec3f(pos.x, pos.y + eps, pos.z), ship) - sceneSDF(Vec3f(pos.x, pos.y - eps, pos.z), ship),
            sceneSDF(Vec3f(pos.x, pos.y, pos.z + eps), ship) - sceneSDF(Vec3f(pos.x, pos.y, pos.z - eps), ship)
    );
    return n.normalized();
}

inline bool raymarchSDF(const Vec3f &orig, const Vec3f &dir, const Ship &ship, float &t) {
    constexpr int max_steps = 100;
    constexpr float max_dist = 100.0f;
    constexpr float epsilon = 0.001f;

    float total_dist = 0.0f;

    for (int i = 0; i < max_steps; ++i) {
        const float dist = sceneSDF(orig + dir * total_dist, ship);

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

inline void draw_shadertoy(TGAImage &image) {
    const int width = image.get_width();
    const int height = image.get_height();

    const Ship ship{Vec3f(0.0f, 0.0f, 5.0f), 0.0f, true};
    const Vec3f light_pos(10.0f, 10.0f, -10.0f);
    const Vec3f orig(0.0f, 0.0f, 0.0f);
    const Vec2f resolution(static_cast<float>(width), static_cast<float>(height));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Vec2f fragCoord(static_cast<float>(x), static_cast<float>(y));
            const Vec2f uv = (fragCoord - resolution * 0.5f) / resolution.y;
            Vec3f dir(uv.x, -uv.y, 1.0f);
            dir = dir.normalized();

            float t;
            if (raymarchSDF(orig, dir, ship, t) && t > 0.0f) {
                const Vec3f hit_pos = orig + dir * t;
                const Vec3f normal = calculateNormal(hit_pos, ship);
                const Vec3f ship_color = getShipColor(hit_pos, ship);
                const Vec3f light_dir = (light_pos - hit_pos).normalized();
                const float intensity = std::max(normal.dot(light_dir), 0.2f);

                const unsigned char r = static_cast<unsigned char>(ship_color.x * intensity * 255.0f);
                const unsigned char g = static_cast<unsigned char>(ship_color.y * intensity * 255.0f);
                const unsigned char b = static_cast<unsigned char>(ship_color.z * intensity * 255.0f);
                image.set(x, y, TGAColor(r, g, b, 255));
            }
        }
    }
}
