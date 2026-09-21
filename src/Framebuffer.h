#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "math/vec3.h"

// Pixel color representation (0 - 255 for RGB channels)
struct ColorRGB {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
};

class Framebuffer {
public:
    Framebuffer(int width, int height);

    // Dimensions
    int width() const;
    int height() const;

    // Pixel access
    void set_pixel(int x, int y, const ColorRGB& color);
    void set_pixel(int x, int y, const vec3& color); // what is Convenience overload for vec3 [0.0, 1.0]
    ColorRGB get_pixel(int x, int y) const;

    // Buffer operations
    void clear(const ColorRGB& clear_color = {0, 0, 0});
    const uint8_t* raw_data() const; // Useful for passing to OpenGL or texture upload

    // Exporting image
    bool write_ppm(const std::string& filepath) const;
    bool write_png(const std::string& filepath) const;

private:
    int m_width;
    int m_height;
    std::vector<ColorRGB> m_pixels; // Flat array: size = m_width * m_height

    // Helper to calculate 1D index from 2D coordinates: index = y * width + x
    int get_index(int x, int y) const;
};
