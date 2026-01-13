#define STB_IMAGE_IMPLEMENTATION

#include "libs/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "libs/stb_image_write.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "renderer.h"
#include "camera.h"
#include "shader.h"
#include "texture.h"
#include "light.h"
#include "libs/tgaimage.h"
#include "shadertoy.h"

namespace {
    constexpr int kWidth = 800;
    constexpr int kHeight = 800;
    const String kModelPath = "FabConvert.com_Isacc/Isacc.obj";
    const String kDiffuseTexturePath = "FabConvert.com_Isacc/IsaccTex.png";

    const String kColorBufferTga = "output.tga";
    const String kColorBufferPng = "output.png";
    const String kDepthBufferTga = "zbuffer.tga";
    const String kDepthBufferPng = "zbuffer.png";

    const Light kLight(Vec3f(0.0f, 1.0f, 1.0f), {1, 1, 1}, 1.5);

    struct Framebuffer {
        Framebuffer()
                : color(kWidth, kHeight, TGAImage::RGB),
                  depth(kWidth * kHeight, std::numeric_limits<float>::lowest()) {
        }

        TGAImage color;
        std::vector<float> depth;
    };

    void render_model(const Model &model, Framebuffer &framebuffer, IShader &shader) {
        for (int face_index = 0; face_index < model.nfaces(); ++face_index) {
            std::array<Vec3f, 3> screen_vertices{};
            for (int vertex_index = 0; vertex_index < 3; ++vertex_index) {
                screen_vertices[vertex_index] = shader.vertex(face_index, vertex_index);
            }

            renderer::barycentric_triangle(
                    screen_vertices,
                    framebuffer.color,
                    framebuffer.depth,
                    shader);
        }
    }

    void dump_depth_buffer(const std::vector<float> &depth, const char *file_path) {
        TGAImage depth_image(kWidth, kHeight, TGAImage::GRAYSCALE);
        for (int x = 0; x < kWidth; ++x) {
            for (int y = 0; y < kHeight; ++y) {
                const int index = x + y * kWidth;
                const float depth_value = depth.at(index);
                const float clamped = std::clamp(depth_value, 0.0f, 1.0f);
                const auto value = static_cast<unsigned char>(clamped * 255.0f);
                depth_image.set(x, y, TGAColor(value, 1));
            }
        }

        depth_image.flip_vertically();
        depth_image.write_tga_file(file_path);
    }

    void convert_tga_to_png(const char *input_path, const char *output_path) {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char *data = stbi_load(input_path, &width, &height, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load " + std::string(input_path));
        }

        const int stride_in_bytes = width * channels;
        const int result = stbi_write_png(output_path, width, height, channels, data, stride_in_bytes);
        stbi_image_free(data);
        if (result == 0) {
            throw std::runtime_error("Failed to write PNG file: " + std::string(output_path));
        }
    }

    void render_rotation_sequence(const Model &model,
                                  const Texture &texture,
                                  const Camera &start_camera,
                                  const String &output_prefix) {
        constexpr int frame_count = 20;
        constexpr float pulse_amplitude = 0.3f;
        constexpr float pulse_cycles = 2.0f;
        const String output_dir = "gif/";

        const Vec3f target = start_camera.get_target();
        const Vec3f start_position = start_camera.get_position();
        const Vec3f to_camera = start_position - target;

        const float base_height = to_camera.y;
        const float base_horizontal_distance = std::sqrt(to_camera.x * to_camera.x + to_camera.z * to_camera.z);
        const float start_angle = std::atan2(to_camera.x, to_camera.z);

        for (int frame = 0; frame < frame_count; ++frame) {
            std::cout << frame << std::endl;
            const float progress = static_cast<float>(frame) / frame_count;
            const float pulse = 1.0f + pulse_amplitude * std::sin(
                    progress * pulse_cycles * 2.0f * std::numbers::pi_v<float>);

            const float angle = start_angle + (2.0f * std::numbers::pi_v<float> * frame) / frame_count;
            const float horizontal_distance = base_horizontal_distance * pulse;
            const float x = std::sin(angle) * horizontal_distance;
            const float z = std::cos(angle) * horizontal_distance;
            const Vec3f new_position = target + Vec3f(x, base_height, z);

            Camera camera(
                    new_position,
                    target,
                    start_camera.get_up(),
                    start_camera.get_fov_deg(),
                    start_camera.get_aspect(),
                    start_camera.get_near_plane(),
                    start_camera.get_far_plane(),
                    start_camera.get_screen_width(),
                    start_camera.get_screen_height());

            const Vec3f light_direction = (target - new_position).normalized();
            const Light light(light_direction, kLight.get_color(), kLight.get_intensity());

            Framebuffer framebuffer{};
            PhongShader shader(model, camera, light, &texture);
            render_model(model, framebuffer, shader);

            framebuffer.color.flip_vertically();
            const String frame_tga = "debug.tga";
            const String frame_png = output_dir + output_prefix + "_" + std::to_string(frame) + ".png";
            framebuffer.color.write_tga_file(frame_tga.c_str());
            convert_tga_to_png(frame_tga.c_str(), frame_png.c_str());
        }
    }
}


// void draw_random_things(TGAImage &picture) {
//     draw_shadertoy(picture);
// }

int main() {
    const Model model(kModelPath);
    const Texture diffuse_texture(kDiffuseTexturePath);
    const Camera camera(
            Vec3f(0.0f, -4.0f, 4.0f),
            Vec3f(0.0f, 0.0f, 3.0f),
            Vec3f(0.0f, 0.0f, 1.0f),
            90.0f,
            static_cast<float>(kWidth) / static_cast<float>(kHeight),
            0.1f,
            100.0f,
            kWidth,
            kHeight);

    // render_rotation_sequence(model, diffuse_texture, camera, "gif");

    Framebuffer framebuffer{};
    PhongShader shader(model, camera, kLight, &diffuse_texture);
    render_model(model, framebuffer, shader);

    framebuffer.color.flip_vertically();
    // draw_random_things(framebuffer.color);
    framebuffer.color.write_tga_file(kColorBufferTga.c_str());
    dump_depth_buffer(framebuffer.depth, kDepthBufferTga.c_str());


    convert_tga_to_png(kColorBufferTga.c_str(), kColorBufferPng.c_str());
    convert_tga_to_png(kDepthBufferTga.c_str(), kDepthBufferPng.c_str());
    return 0;
}
