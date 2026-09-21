#include <iostream>
#include "Framebuffer.h"
#include "math/vec3.h"

int main() {
    const int image_width = 256;
    const int image_height = 256;

    Framebuffer fb(image_width, image_height);

    std::cout << "Rendering image..." << std::endl;

    // Render loop: scan through rows from top to bottom
    for (int y = 0; y < image_height; ++y) {
        for (int x = 0; x < image_width; ++x) {
            // Normalize coordinates to [0.0, 1.0]
            double r = double(x) / (image_width - 1);
            double g = double(y) / (image_height - 1);
            double b = 0.25;

            fb.set_pixel(x, y, vec3(r, g, b));
        }
    }

    const std::string output_filename = "output.ppm";
    if (fb.write_ppm(output_filename)) {
        std::cout << "Successfully saved render to " << output_filename << std::endl;
    } else {
        std::cerr << "Failed to write image file!" << std::endl;
        return 1;
    }

    return 0;
}
