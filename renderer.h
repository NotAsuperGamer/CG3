#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "geometry.h"
#include "libs/tgaimage.h"
#include "shader.h"

namespace renderer
{
    inline void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color)
    {
        bool steep = false;

        if (std::abs(x0 - x1) < std::abs(y0 - y1))
        {
            std::swap(x0, y0);
            std::swap(x1, y1);
            steep = true;
        }

        if (x0 > x1)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        const int dx = x1 - x0;
        const int dy = std::abs(y1 - y0);
        int error = dx / 2;
        const int y_step = (y0 < y1) ? 1 : -1;
        int y = y0;

        for (int x = x0; x <= x1; ++x)
        {
            if (steep)
            {
                image.set(y, x, color);
            }
            else
            {
                image.set(x, y, color);
            }

            error -= dy;
            if (error < 0)
            {
                y += y_step;
                error += dx;
            }
        }
    }

    inline void line(const Vec2i& p0, const Vec2i& p1, TGAImage& image, const TGAColor& color)
    {
        line(p0.x, p0.y, p1.x, p1.y, image, color);
    }

    inline void triangle_wireframe(const Vec2i& t0, const Vec2i& t1, const Vec2i& t2,
                                   TGAImage& image, const TGAColor& color)
    {
        line(t0, t1, image, color);
        line(t1, t2, image, color);
        line(t2, t0, image, color);
    }

    inline void draw_scanline(TGAImage& image, int x0, int x1, int y, const TGAColor& color)
    {
        if (x0 > x1)
        {
            std::swap(x0, x1);
        }

        for (int x = x0; x <= x1; ++x)
        {
            image.set(x, y, color);
        }
    }

    inline void colored_triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, const TGAColor& color)
    {
        if (t0.y > t1.y) std::swap(t0, t1);
        if (t0.y > t2.y) std::swap(t0, t2);
        if (t1.y > t2.y) std::swap(t1, t2);

        const float dx02 = static_cast<float>(t2.x - t0.x) / static_cast<float>(t2.y - t0.y);
        const float dx01 = static_cast<float>(t1.x - t0.x) / static_cast<float>(t1.y - t0.y);
        const float dx12 = static_cast<float>(t2.x - t1.x) / static_cast<float>(t2.y - t1.y);

        float x02 = static_cast<float>(t0.x);
        float x01 = static_cast<float>(t0.x);

        for (int y = t0.y; y < t1.y; ++y)
        {
            draw_scanline(image, static_cast<int>(x02), static_cast<int>(x01), y, color);
            x02 += dx02;
            x01 += dx01;
        }

        float x12 = static_cast<float>(t1.x);
        for (int y = t1.y; y <= t2.y; ++y)
        {
            draw_scanline(image, static_cast<int>(x02), static_cast<int>(x12), y, color);
            x02 += dx02;
            x12 += dx12;
        }
    }

    inline void barycentric_triangle(const std::array<Vec3f, 3>& screen_vertices,
                                     TGAImage& image,
                                     std::vector<float>& zbuffer,
                                     IShader& shader)
    {
        const int width = image.get_width();
        const int height = image.get_height();

        const int min_x = std::clamp(static_cast<int>(std::floor(std::min({
                                         screen_vertices[0].x, screen_vertices[1].x, screen_vertices[2].x
                                     }))), 0, width - 1);
        const int max_x = std::clamp(static_cast<int>(std::ceil(std::max({
                                         screen_vertices[0].x, screen_vertices[1].x, screen_vertices[2].x
                                     }))), 0, width - 1);
        const int min_y = std::clamp(static_cast<int>(std::floor(std::min({
                                         screen_vertices[0].y, screen_vertices[1].y, screen_vertices[2].y
                                     }))), 0, height - 1);
        const int max_y = std::clamp(static_cast<int>(std::ceil(std::max({
                                         screen_vertices[0].y, screen_vertices[1].y, screen_vertices[2].y
                                     }))), 0, height - 1);

        const Vec2f v0(screen_vertices[1].x - screen_vertices[0].x,
                       screen_vertices[1].y - screen_vertices[0].y);
        const Vec2f v1(screen_vertices[2].x - screen_vertices[0].x,
                       screen_vertices[2].y - screen_vertices[0].y);

        const float dot00 = v0.x * v0.x + v0.y * v0.y;
        const float dot01 = v0.x * v1.x + v0.y * v1.y;
        const float dot11 = v1.x * v1.x + v1.y * v1.y;
        const float denom = dot00 * dot11 - dot01 * dot01;

        if (std::abs(denom) < 1e-6f)
        {
            return;
        }

        const float inv_denom = 1.0f / denom;

        for (int y = min_y; y <= max_y; ++y)
        {
            for (int x = min_x; x <= max_x; ++x)
            {
                const Vec2f v2(static_cast<float>(x) - screen_vertices[0].x,
                               static_cast<float>(y) - screen_vertices[0].y);
                const float dot02 = v0.x * v2.x + v0.y * v2.y;
                const float dot12 = v1.x * v2.x + v1.y * v2.y;

                const float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
                const float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

                if (u < 0.0f || v < 0.0f || (u + v) > 1.0f)
                {
                    continue;
                }

                const float w = 1.0f - u - v;
                const float z = screen_vertices[0].z * w +
                    screen_vertices[1].z * u +
                    screen_vertices[2].z * v;

                const int index = x + y * width;
                if (z > zbuffer.at(index))
                {
                    TGAColor color;
                    if (!shader.fragment(Vec3f(w, u, v), color))
                    {
                        zbuffer.at(index) = z;
                        image.set(x, y, color);
                    }
                }
            }
        }
    }
}
