#pragma once
#include <algorithm>
// ============================================================================
// DebugFramebuffer.h  —  Drop-in Framebuffer wrapper with live visualization
//
// This wraps your existing Framebuffer so that every set_pixel call is also
// streamed to the Python viewer.  It also captures ray/hit info if you're
// using shoot_ray from a Camera.
//
// USAGE (automatic when -DDEBUG_VIS=ON):
//   Your code keeps using "Framebuffer" — a typedef swaps it to this wrapper.
//   No changes to your render code needed.
//
// MANUAL USAGE (if you want explicit control):
//   #include "tools/debugvis/DebugFramebuffer.h"
//   DebugFramebuffer fb(400, 300);   // use exactly like Framebuffer
// ============================================================================

#include "Framebuffer.h"
#include "DebugVisualizer.h"

class DebugFramebuffer {
public:
    DebugFramebuffer(int width, int height)
        : m_fb(width, height) {
        DebugVisualizer::instance().init(width, height);
    }

    // ---- Forward all Framebuffer methods ----

    int width()  const { return m_fb.width(); }
    int height() const { return m_fb.height(); }

    void set_pixel(int x, int y, const ColorRGB& color) {
        m_fb.set_pixel(x, y, color);
        DebugVisualizer::instance().pixel(x, y, color.r, color.g, color.b);
    }

    void set_pixel(int x, int y, const vec3& color) {
        // Convert to 0-255 to send the debug info, then forward
        uint8_t r = static_cast<uint8_t>(255.999 * std::clamp(color.x(), 0.0f, 1.0f));
        uint8_t g = static_cast<uint8_t>(255.999 * std::clamp(color.y(), 0.0f, 1.0f));
        uint8_t b = static_cast<uint8_t>(255.999 * std::clamp(color.z(), 0.0f, 1.0f));
        m_fb.set_pixel(x, y, color);
        DebugVisualizer::instance().pixel(x, y, r, g, b);
    }

    ColorRGB get_pixel(int x, int y) const { return m_fb.get_pixel(x, y); }

    void clear(const ColorRGB& clear_color = {0, 0, 0}) {
        m_fb.clear(clear_color);
    }

    bool write_png(const std::string& filepath) const {
        DebugVisualizer::instance().frame_done();
        return m_fb.write_png(filepath);
    }

    // ---- Extra debug helpers you can optionally call ----

    // Log ray info for a specific pixel (call right after shoot_ray)
    void debug_ray(int px, int py, const ray& r) {
        DebugVisualizer::instance().ray_info(
            px, py,
            static_cast<float>(r.origin().x()),
            static_cast<float>(r.origin().y()),
            static_cast<float>(r.origin().z()),
            static_cast<float>(r.direction().x()),
            static_cast<float>(r.direction().y()),
            static_cast<float>(r.direction().z())
        );
    }

    // Log hit info for a specific pixel
    void debug_hit(int px, int py, double t,
                   const vec3& normal, const vec3& hit_point) {
        DebugVisualizer::instance().hit_info(
            px, py, static_cast<float>(t),
            static_cast<float>(normal.x()),
            static_cast<float>(normal.y()),
            static_cast<float>(normal.z()),
            static_cast<float>(hit_point.x()),
            static_cast<float>(hit_point.y()),
            static_cast<float>(hit_point.z())
        );
    }

    // Signal that a scanline finished rendering
    void debug_row_done(int y) {
        DebugVisualizer::instance().row_done(y);
    }

    // Get access to underlying Framebuffer if needed
    Framebuffer&       inner()       { return m_fb; }
    const Framebuffer& inner() const { return m_fb; }

    // Implicit conversion so it can be passed where Framebuffer& is expected
    operator Framebuffer&()             { return m_fb; }
    operator const Framebuffer&() const { return m_fb; }

private:
    Framebuffer m_fb;
};
