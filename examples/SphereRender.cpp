#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"
#include "shapes/Circle.h" //well I suppose its a sphere lol

#include <iostream>
#include <string>

void render_flag(const std::string& name, const std::string& filename,
                 int width, float aspect_ratio,
                 const point3& sphere_pos, double sphere_rad,
                 const color& sphere_col, const color& bg_col) {
    const float focal_length = 1.0f;
    const float viewport_height = 2.0f;
    const point3 camera_position(0.0, 0.0, 0.0);
    const vec3 camera_rotation(0.0, 0.0, 0.0);

    perspective_camera cam(width, aspect_ratio, focal_length, viewport_height, camera_position, camera_rotation);
    Framebuffer fb(cam.image_width, cam.image_height);

    std::cout << "Rendering " << name << " (" << cam.image_width << "x" << cam.image_height << ") to " << filename << "...\n";

    Sphere s(sphere_pos, sphere_rad);

    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) { //ayo ++x what the heck is that the way its supposed to be x++???
            ray r = cam.generateRay(x, y); //? generate ray

            if (s.intersect(r)) {
                fb.set_pixel(x, y, sphere_col);
            } else {
                fb.set_pixel(x, y, bg_col);
            }
        }
    }

    fb.write_png(filename);
}

int main() {
    // 1. Bangladeshi Flag (Default as requested in lab instructions, ratio 5:3)
    // Field: Bottle green (#006a4e -> RGB: 0, 106, 78)
    // Disc: Vibrant red (#f42a41 -> RGB: 244, 42, 65)
    // Disc radius: 1/5th of width, shifted slightly hoist-side by 5% of width
    const float bg_aspect = 5.0f / 3.0f;
    const float bg_v_width = 2.0f * bg_aspect;
    render_flag("Bangladeshi Flag", "output.png", 600, bg_aspect,
                point3(-0.05 * bg_v_width, 0.0, -1.0), (1.0 / 5.0) * bg_v_width,
                color(244.0 / 255.0, 42.0 / 255.0, 65.0 / 255.0),
                color(0.0, 106.0 / 255.0, 78.0 / 255.0));

    std::clog << "\nThe flag successfully rendered to PNG!\n";
    return 0;
}
