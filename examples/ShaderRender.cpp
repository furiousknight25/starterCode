#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"
#include "shapes/ShapeList.h"
#include "Shader.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

void render_scene(const std::string& filename, const Scene& scene, int width, int height) {
    perspective_camera cam(width, height, 1.0f, 2.0f, point3(0, 0, 0), vec3(0, 0, 0));
    Framebuffer fb(cam.image_width, cam.image_height);

    std::cout << "Rendering " << filename << "...\n";

    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            ray r = cam.generateRay(x, y);
            color pixel_color = scene.computeRayColor(r, color(0.1, 0.1, 0.15));
            fb.set_pixel(x, y, pixel_color);
        }
    }

    fb.write_png(filename);
}

int main() {
    point3 light_pos(0.0, 10.0, 5.0); // from lab instructions
    color red(0.85, 0.25, 0.25);

    // 1. Lambertian sphere (matte)
    Scene lambert_scene;
    auto lambert = std::make_shared<Lambertian>(red, light_pos);
    lambert_scene.add(std::make_shared<Sphere>(point3(0, 0, -2), 0.7, lambert));
    render_scene("lambertian_sphere.png", lambert_scene, 400, 400);

    // 2. Blinn-Phong sphere (shiny with specular highlight)
    Scene bp_scene;
    auto bp = std::make_shared<BlinnPhong>(red, 64.0, color(1.0, 1.0, 1.0), light_pos);
    bp_scene.add(std::make_shared<Sphere>(point3(0, 0, -2), 0.7, bp));
    render_scene("blinn_phong_sphere.png", bp_scene, 400, 400);

    // 3. Grid of 9 spheres with different exponents (from slide)
    Scene grid_scene;
    std::vector<double> exps = {2, 4, 8, 16, 32, 64, 128, 256, 512};
    double xs[] = {-0.65, 0.0, 0.65};
    double ys[] = {0.65, 0.0, -0.65};

    int count = 0;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            auto s = std::make_shared<BlinnPhong>(red, exps[count++], color(1, 1, 1), light_pos);
            grid_scene.add(std::make_shared<Sphere>(point3(xs[c], ys[r], -2.2), 0.26, s));
        }
    }
    render_scene("blinn_phong_9spheres.png", grid_scene, 500, 500);

    std::cout << "Done rendering!\n";
    return 0;
}
