#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"

#include <iostream>

// Maps ray unit direction [-1.0, 1.0] to RGB color channels [0.0, 1.0]
color ray_color(const ray& r) {
    vec3 unitvec = unit_vector(r.direction());
    return 0.5 * (unitvec + color(1.0, 1.0, 1.0));
}

int main() {
    // Image and Camera setup
    auto aspect_ratio = 1.0;
    int image_width = 200;
    float focal_length = 0.5;

    // Create camera with a new position and rotation
    // Position: camera elevated at (0, 1, 0)
    // Rotation: pitch = 15 deg (looking slightly up), yaw = 30 deg (turned right), roll = 45 deg (tilted 45 deg)
    // The rotation angles change the direction vectors of the raycasts, producing a dynamic diagonal gradient
    point3 camera_position(1.0, 3.0, 6.0);
    vec3 camera_rotation(-2.0, -1.5, -4.0);

    perspective_camera cam(image_width, aspect_ratio, camera_position, camera_rotation, focal_length);
    Framebuffer fb(cam.image_width, cam.image_height);

    std::cout << "Rendering scene with camera position: " << cam.camera_center << "\n";
    std::cout << "Camera rotation (pitch, yaw, roll in deg): " << cam.rotation << "\n";

    // Render
    for (int j = 0; j < cam.image_height; j++) {
        std::clog << "\rScanlines remaining: " << (cam.image_height - j) << ' ' << std::flush;
        for (int i = 0; i < cam.image_width; i++) {
            ray r = cam.generateRay(i, j);
            color pixel_color = ray_color(r);
            fb.set_pixel(i, j, pixel_color);
        }
    }

    fb.write_png("output.png");
    std::clog << "\rDone rendering to output.png.       \n";
}
