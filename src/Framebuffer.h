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

    bool operator==(const ColorRGB&) const = default;
};

class Framebuffer {
public:
    Framebuffer(int width, int height);

    int width() const;
    int height() const;

    void set_pixel(int x, int y, const ColorRGB& color);
    void set_pixel(int x, int y, const vec3& color);
    ColorRGB get_pixel(int x, int y) const;

    void clear(const ColorRGB& clear_color = {0, 0, 0});
    bool write_png(const std::string& filepath) const;

private:
    int m_width;
    int m_height;
    std::vector<ColorRGB> m_pixels;
};
