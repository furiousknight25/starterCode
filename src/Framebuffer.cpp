#include "Framebuffer.h"
#include <fstream>
#include <algorithm>
#include "png++/png.hpp"

Framebuffer::Framebuffer(int width, int height) //what is this syntax?
    : m_width(width), m_height(height), m_pixels(width * height) {}

int Framebuffer::width() const {
    return m_width;
}

int Framebuffer::height() const {
    return m_height;
}

int Framebuffer::get_index(int x, int y) const {
    return y * m_width + x;
}

void Framebuffer::set_pixel(int x, int y, const ColorRGB& color) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return; // Guard against out-of-bounds writes
    }
    m_pixels[get_index(x, y)] = color;
}

void Framebuffer::set_pixel(int x, int y, const vec3& color) {
    // Clamp [0.0, 1.0] float/double values and convert to [0, 255] byte values
    auto clamp_channel = [](double val) -> uint8_t { //hm what is this whole section
        val = std::clamp(val, 0.0, 0.999);
        return static_cast<uint8_t>(256 * val);
    };

    ColorRGB rgb{
        clamp_channel(color.x()),
        clamp_channel(color.y()),
        clamp_channel(color.z())
    };

    set_pixel(x, y, rgb);
}

ColorRGB Framebuffer::get_pixel(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return {0, 0, 0};
    }
    return m_pixels[get_index(x, y)];
}

void Framebuffer::clear(const ColorRGB& clear_color) {
    std::fill(m_pixels.begin(), m_pixels.end(), clear_color); //std::fill looks to fill the "array" with something, in this case nothing
}

const uint8_t* Framebuffer::raw_data() const { //woah what the heck is this syntax
    return reinterpret_cast<const uint8_t*>(m_pixels.data());
}

bool Framebuffer::write_ppm(const std::string& filepath) const {
    std::ofstream out(filepath); //of stream is used to write to files, out is just the datapath
    if (!out.is_open()) {
        return false;
    }

    // Write standard ASCII PPM header (P3, width, height, max color value)
    out << "P3\n" << m_width << ' ' << m_height << "\n255\n";

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            ColorRGB pixel = get_pixel(x, y);
            out << static_cast<int>(pixel.r) << ' '
                << static_cast<int>(pixel.g) << ' '
                << static_cast<int>(pixel.b) << '\n';
        }
    }

    return true;
}

bool Framebuffer::write_png(const std::string& filepath) const {
    try {
        png::image<png::rgb_pixel> image(m_width, m_height);
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                ColorRGB pixel = get_pixel(x, y);
                image[y][x] = png::rgb_pixel(pixel.r, pixel.g, pixel.b);
            }
        }
        image.write(filepath);
        return true;
    } catch (...) {
        return false;
    }
}
