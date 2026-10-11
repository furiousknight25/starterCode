#include "Framebuffer.cpp"
#include <cmath>

int main() {
    Framebuffer fb(100, 100);
    
    for (int x = 0; x < fb.height(); x++) {

    for (int y = 0; y < fb.width(); y++) {
        double s_val = 0.5 * (std::sin(x * .1) + 1.0); // made sure the sin value is mapped to color range
        fb.set_pixel(x, y, vec3(s_val,s_val,s_val));
    }
    }
    
    fb.write_png("scratchrender");
}