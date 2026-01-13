#pragma once

#include <vector>
#include "geometry.h"

class Model
{
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<int>> faces_;

    std::vector<Vec2f> texcoords_;
    std::vector<std::vector<int>> texcoord_indices_;

public:
    void parse_face(std::istringstream& iss);
    void parse_vertex(std::istringstream& iss);
    void parse_texcoord(std::istringstream& iss);
    void parse_file(std::ifstream& in);
    explicit Model(const String& filename);
    ~Model();
    [[nodiscard]] size_t nverts() const;
    [[nodiscard]] size_t nfaces() const;
    [[nodiscard]] const Vec3f& vert(int i) const;
    [[nodiscard]] const std::vector<int>& face(int idx) const;
    [[nodiscard]] Vec2f texcoord(int face_index, int vertex_index) const;
    [[nodiscard]] bool has_texcoords() const { return !texcoords_.empty(); }
    [[nodiscard]] Vec3f normal(int vertex_index) const;

private:
    void compute_vertex_normals();
    std::vector<Vec3f> vertex_normals_;
};
