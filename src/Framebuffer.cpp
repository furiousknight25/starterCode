#include "Framebuffer.h"
#include <algorithm>
#include "png++/png.hpp"


Framebuffer::Framebuffer(int width, int height) //creates variables here
    : m_width(width), m_height(height), m_pixels(width * height) {}
    
//reminder the width was defined in the header, compiler needs to know ahead of time that it exists?
//also reminder that other classes only extend the header, due to this I believe they are all linked to this
int Framebuffer::width() const {    
    return m_width; 
}

int Framebuffer::height() const {
    return m_height; //member variable, variable that belongs to the class
}

void Framebuffer::set_pixel(int x, int y, const ColorRGB& color) { //interesting, so const prevents it from changing the reference but the & reference speeds up time as you are simply passing a reference instead of copying the variable
    m_pixels[y * m_width + x]= color; //reach the end of a row with the width then walk with x
}

void Framebuffer::set_pixel(int x, int y, const vec3& color) {
    // helper to clamp between 0.0 and 1.0 and convert to 0-255 byte
    int r = static_cast<int>(255.999 * std::clamp(color.x(), 0.0f, 1.0f));
    int g = static_cast<int>(255.999 * std::clamp(color.y(), 0.0f, 1.0f));
    int b = static_cast<int>(255.999 * std::clamp(color.z(), 0.0f, 1.0f));
    set_pixel(x, y, ColorRGB(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)));
}

ColorRGB Framebuffer::get_pixel(int x, int y) const {
    return m_pixels[y * m_width + x];
}

void Framebuffer::clear(const ColorRGB& clear_color) { //default value already made in the header
    for (int i = 0; i < m_pixels.size(); i++) {
        m_pixels[i] = clear_color;
    }
}

bool Framebuffer::write_png(const std::string& filepath) const { //std::string allocates this text onto the heap? but why? and the & makes it a reference, but why do that here? why not just send in a regular string
    png::image<png::rgb_pixel> image(m_width, m_height); //is this what makes the object?
    for (int y = 0; y <= m_height -1 ; y++) {
        for (int x = 0; x <= m_width -1 ; x++) {
            ColorRGB pix = get_pixel(x,y);
            image[y][x] = png::rgb_pixel(pix.r,pix.g,pix.b);
           }
    }
    try {
        image.write(filepath);
        return true;
    }
    catch(const std::exception& e) { return false; }
}