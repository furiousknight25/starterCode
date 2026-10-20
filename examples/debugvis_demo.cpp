// ============================================================================
// debugvis_demo.cpp  —  Minimal demo of the debug visualizer
//
// BUILD:
//   cd buildVCPkg
//   cmake -DDEBUG_VIS=ON ..
//   make debugvis_demo
//
// RUN:
//   ../tools/debugvis/debugvis.sh ./examples/debugvis_demo
//
// This renders a simple gradient + sphere-like pattern and streams
// every pixel + ray to the live viewer.
// ============================================================================

#include "Framebuffer.h"
#include "math/ray.h"
#include "math/vec3.h"

// Conditionally include the debug wrapper
#ifdef DEBUGVIS_ENABLED
  #include "DebugFramebuffer.h"
  using FB = DebugFramebuffer;
#else
  using FB = Framebuffer;
#endif

#include <cmath>
#include <iostream>

// Simple sphere intersection for demo purposes
bool hit_sphere(const point3& center, double radius, const ray& r,
                double& t_out, vec3& normal_out) {
    vec3 oc = r.origin() - center;
    double a = dot(r.direction(), r.direction());
    double b = 2.0 * dot(oc, r.direction());
    double c = dot(oc, oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;
    t_out = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if (t_out < 0) return false;
    point3 hit_p = r.at(t_out);
    normal_out = normalize(hit_p - center);
    return true;
}

int main() {
    const int W = 200, H = 150;
    FB fb(W, H);

    // Camera setup
    point3 cam_origin(0, 0, 0);
    double focal_length = 1.0;
    double viewport_h = 2.0;
    double viewport_w = viewport_h * (double(W) / H);

    // Sphere
    point3 sphere_center(0, 0, -3);
    double sphere_radius = 1.0;

    std::cout << "Rendering " << W << "x" << H << " with debug vis..." << std::endl;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            // Map pixel to viewport
            double u = (double(x) / (W - 1)) * viewport_w - viewport_w / 2.0;
            double v = (double(y) / (H - 1)) * viewport_h - viewport_h / 2.0;

            vec3 direction(u, -v, -focal_length);
            ray r(cam_origin, direction);

            // ── Optional: send ray info to viewer ──
            #ifdef DEBUGVIS_ENABLED
            fb.debug_ray(x, y, r);
            #endif

            // Shade
            double t;
            vec3 normal;
            color pixel_color;

            if (hit_sphere(sphere_center, sphere_radius, r, t, normal)) {
                // Normal-mapped coloring
                pixel_color = 0.5 * (normal + vec3(1, 1, 1));

                #ifdef DEBUGVIS_ENABLED
                fb.debug_hit(x, y, t, normal, r.at(t));
                #endif
            } else {
                // Sky gradient
                vec3 unit_dir = normalize(direction);
                double blend = 0.5 * (unit_dir.y() + 1.0);
                pixel_color = (1.0 - blend) * color(1, 1, 1) + blend * color(0.5, 0.7, 1.0);
            }

            fb.set_pixel(x, y, pixel_color);
        }

        #ifdef DEBUGVIS_ENABLED
        fb.debug_row_done(y);
        #endif
    }

    fb.write_png("debugvis_demo.png");
    std::cout << "Done! Wrote debugvis_demo.png" << std::endl;
    return 0;
}
