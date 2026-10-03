#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"
#include "shapes/ShapeList.h"

#include <iostream>
#include <memory>
#include <random>
#include <string>

// Pseudo-random uniform real in [0, 1)
inline double random_double() {
    static thread_local std::mt19937 generator(1337);
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

// Builds the multi-shape scene:
// - Normal visualization on spheres
// - Flat colors on triangles
ShapeList build_multishape_scene() {
    ShapeList world;

    //sphere
    auto sphere = std::make_shared<Sphere>(point3(-0.9, 0.2, -2.8), 0.35);
    sphere ->use_normal_coloring = true;
    world.add(sphere);

    //close triangle
    world.add(std::make_shared<Triangle>(
        point3(0.15, -0.4, -1.8),
        point3(1.05, -0.4, -1.8),
        point3(0.60, 0.55, -1.8),
        color(0.95, 0.75, 0.10)
    ));

    //overlap triangle
    world.add(std::make_shared<Triangle>(
        point3(-1.6, -0.1, -4.0),
        point3(-0.4, -0.1, -4.0),
        point3(-1.0, 1.25, -4.0),
        color(0.60, 0.20, 0.80)
    ));

    return world;
}


// Flat background color (light sky blue)
const color FLAT_BG_COLOR(0.65, 0.82, 0.95);

void render_image(const std::string& filename, const ShapeList& world,
                  perspective_camera& cam, int samples_per_pixel) {
    Framebuffer fb(cam.image_width, cam.image_height);

    std::cout << "Rendering " << filename << " (" << cam.image_width << "x" << cam.image_height
              << ", " << samples_per_pixel << " spp)...\n";

    for (int j = 0; j < cam.image_height; ++j) {
        for (int i = 0; i < cam.image_width; ++i) {
            color pixel_color(0.0, 0.0, 0.0);

            for (int s = 0; s < samples_per_pixel; ++s) {
                float u_offset = (samples_per_pixel > 1) ? static_cast<float>(random_double() - 0.5) : 0.0f;
                float v_offset = (samples_per_pixel > 1) ? static_cast<float>(random_double() - 0.5) : 0.0f;

                ray r = cam.generateRay(static_cast<float>(i) + u_offset, static_cast<float>(j) + v_offset);

                hit_record rec;
                if (world.intersect(r, 0.001, 1000.0, rec)) {
                    if (rec.is_sphere) {
                        // Normal visualization on spheres: map [-1, 1] normal to [0, 1] RGB
                        vec3 n = unit_vector(rec.normal);
                        pixel_color += 0.5 * (n + color(1.0, 1.0, 1.0));
                    } else {
                        // Flat colors on triangles
                        pixel_color += rec.mat_color;
                    }
                } else {
                    pixel_color += FLAT_BG_COLOR;
                }
            }

            pixel_color /= static_cast<double>(samples_per_pixel);
            fb.set_pixel(i, j, pixel_color);
        }
    }

    fb.write_png(filename);
    std::cout << "  Saved to " << filename << "\n";
}

int main() {
    // Homework requirements: 200x200 resolution
    const int width = 200;
    const int height = 200;
    const float aspect_ratio = 1.0f;
    const float focal_length = 1.0f;
    const float viewport_height = 2.0f;
    const point3 camera_pos(0.0, 0.0, 0.0);
    const vec3 camera_rot(0.0, 0.0, 0.0);

    perspective_camera cam(width, height, focal_length, viewport_height, camera_pos, camera_rot);

    ShapeList multishape_scene = build_multishape_scene();

    render_image("scene_multishape_200x200.png", multishape_scene, cam, 16);

    render_image("scene_aliased_1spp_200x200.png", multishape_scene, cam, 1);

    std::cout << "\nAll images successfully generated at 200x200!\n";
    return 0;
}
