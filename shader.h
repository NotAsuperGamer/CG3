#pragma once

#include <array>

#include "camera.h"
#include "geometry.h"
#include "light.h"
#include "libs/tgaimage.h"
#include "model.h"
#include "texture.h"

class IShader
{
public:
    virtual ~IShader() = default;

    virtual Vec3f vertex(int face_index, int vertex_index) = 0;
    virtual bool fragment(const Vec3f& barycentric, TGAColor& color) = 0;
};

class BasicShader : public IShader
{
public:
    BasicShader(const Model& model,
                const Camera& camera,
                const Light& light,
                const Texture* texture);

    Vec3f vertex(int face_index, int vertex_index) override;
    bool fragment(const Vec3f& barycentric, TGAColor& color) override;

private:
    const Model& model_;
    const Camera& camera_;
    const Light& light_;
    const Texture* texture_;

    std::array<Vec3f, 3> world_coords_;
    std::array<Vec3f, 3> screen_coords_;
    std::array<Vec2f, 3> uv_coords_;
};

class PhongShader : public IShader
{
public:
    PhongShader(const Model& model,
                const Camera& camera,
                const Light& light,
                const Texture* texture,
                float ambient_strength = 0.1f,
                float specular_strength = 0.5f,
                float shininess = 32.0f);

    Vec3f vertex(int face_index, int vertex_index) override;
    bool fragment(const Vec3f& barycentric, TGAColor& color) override;

private:
    const Model& model_;
    const Camera& camera_;
    const Light& light_;
    const Texture* texture_;

    float ambient_strength_;
    float specular_strength_;
    float shininess_;

    std::array<Vec3f, 3> world_coords_;
    std::array<Vec3f, 3> screen_coords_;
    std::array<Vec2f, 3> uv_coords_;
    std::array<Vec3f, 3> normals_;
};
