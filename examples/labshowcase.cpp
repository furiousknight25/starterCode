#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"
#include "shapes/ShapeList.h"
#include "Shader.h"

#include <iostream>
#include <memory>
#include <random>
#include <string>

// Pseudo-random uniform real in [0, 1)
inline double random_double() {
    static thread_local std::mt19937 generator(1337);
    static std::uniform_real_distribution<double> distribution(0.0, 1.0); //not really sure how the random stuff works here, this will most likely stay generated for now
    return distribution(generator);
}

ShapeList build_test_scene() {
    ShapeList scene;

    point3 light_pos(-5.0, 8.0, 5.0);

    // Shaders
    //
    // Lambertian
    auto ground_shader = std::make_shared<Lambertian>(color(0.6, 0.6, 0.6), light_pos);
    auto yellow_shader = std::make_shared<Lambertian>(color(0.9, 0.8, 0.1), light_pos);
    auto green_shader = std::make_shared<Lambertian>(color(0.2, 0.8, 0.2), light_pos);
    auto purple_shader = std::make_shared<Lambertian>(color(0.6, 0.2, 0.8), light_pos);

    // Blinn-Phong 
    auto red_shader = std::make_shared<BlinnPhong>(color(0.8, 0.1, 0.1), 64.0, color(1.0, 1.0, 1.0), light_pos);
    auto blue_shader = std::make_shared<BlinnPhong>(color(0.1, 0.2, 0.8), 16.0, color(1.0, 1.0, 1.0), light_pos);
    auto orange_shader = std::make_shared<BlinnPhong>(color(0.95, 0.5, 0.1), 32.0, color(1.0, 1.0, 1.0), light_pos);

    // Normal visualization shader
    auto normal_shader      = std::make_shared<NormalShader>();

    // Triangles
    // Ground1 (grey diffuse)
    scene.add(std::make_shared<Triangle>(
        point3(-50.0, 0.0,  50.0),
        point3( 50.0, 0.0,  50.0),
        point3( 50.0, 0.0, -50.0),
        ground_shader
    ));

    // Ground2 (grey diffuse)
    scene.add(std::make_shared<Triangle>(
        point3(-50.0, 0.0,  50.0),
        point3( 50.0, 0.0, -50.0),
        point3(-50.0, 0.0, -50.0),
        ground_shader
    ));

    // FloatingTriangle (yellow diffuse)
    scene.add(std::make_shared<Triangle>(
        point3(0.9, 1.4, -0.6),
        point3(1.9, 1.4, -0.6),
        point3(1.4, 2.2, -1.1),
        yellow_shader
    ));

    // Spheres
    // GreenSphere: Lambertian green (.2 .8 .2)
    scene.add(std::make_shared<Sphere>(point3(-2.0, 0.8, -3.0), 0.8, green_shader));

    // RedSphere: Blinn-Phong red, phong exponent 64
    scene.add(std::make_shared<Sphere>(point3(-1.1, 0.6, -1.2), 0.6, red_shader));

    // NormalSphere: Normal shader
    scene.add(std::make_shared<Sphere>(point3(0.0, 0.9, -3.5), 0.9, normal_shader));

    // BlueSphere: Blinn-Phong blue, phong exponent 16
    scene.add(std::make_shared<Sphere>(point3(0.3, 0.45, -1.6), 0.45, blue_shader));

    // Spheres behind the camera
    // OrangeSphere: center: 0.3 1 5, radius: 1, phong exponent 32
    scene.add(std::make_shared<Sphere>(point3(0.3, 1.0, 5.0), 1.0, orange_shader));

    // PurpleSphere: center: 2.4 0.8 4.8, radius: 0.8, diffuse purple
    scene.add(std::make_shared<Sphere>(point3(2.4, 0.8, 4.8), 0.8, purple_shader));

    return scene;
}

int main() {
    const int width = 500;
    const int height = 500;
    const color bg_color(0.5, 0.7, 1.0); 

    const float focal_length = 0.4f;
    const float viewport_height = 0.5f;
    const point3 camera_pos(0.0, 3.0, 3.0);
    const vec3 view_dir(0.0, -0.4, -1.0);

    perspective_camera cam(width, height, focal_length, viewport_height, camera_pos);
    cam.set_lookat(camera_pos + view_dir);

    ShapeList scene = build_test_scene();
    Framebuffer fb(width, height);

    // Anti-aliasing: 10 x 10 samples
    const int grid_size = 10;
    const int total_samples = grid_size * grid_size;

    std::cout << "Rendering test_scene in labshowcase (" << width << "x" << height
              << ", " << total_samples << " jittered samples per pixel)...\n";

    for (int j = 0; j < height; ++j) {
        if (j % 50 == 0 || j == height - 1) {
            std::cout << "  Row " << j << " / " << height << "\n";
        }

        for (int i = 0; i < width; ++i) {
            color pixel_color(0.0, 0.0, 0.0);

            for (int gy = 0; gy < grid_size; ++gy) {
                for (int gx = 0; gx < grid_size; ++gx) {
                    // Jittered sub-pixel offset within [-0.5, 0.5]
                    float u_offset = (static_cast<float>(gx) + static_cast<float>(random_double())) / grid_size - 0.5f;
                    float v_offset = (static_cast<float>(gy) + static_cast<float>(random_double())) / grid_size - 0.5f;

                    ray r = cam.generateRay(static_cast<float>(i) + u_offset, static_cast<float>(j) + v_offset);
                    pixel_color += scene.computeRayColor(r, bg_color);
                }
            }

            pixel_color /= static_cast<double>(total_samples);
            fb.set_pixel(i, j, pixel_color);
        }
    }

    const std::string output_file = "labshowcase.png";
    fb.write_png(output_file);
    std::cout << "Successfully rendered and saved to " << output_file << "!\n";

    return 0;
}
