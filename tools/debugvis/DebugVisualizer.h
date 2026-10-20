#pragma once
// ============================================================================
// DebugVisualizer.h  —  Zero-overhead debug probe for your raytracer
//
// HOW IT WORKS
//   A singleton connects (lazily, on first use) to a Python viewer over a
//   Unix-domain socket.  Every call streams a tiny binary message.  If the
//   viewer isn't running, every call is a no-op (one cheap branch).
//
// YOU NEVER NEED TO INCLUDE THIS DIRECTLY.
//   When CMake option  DEBUG_VIS=ON  is set, DebugFramebuffer.h transparently
//   wraps your Framebuffer so every set_pixel is mirrored to the viewer.
// ============================================================================

#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

// Message type tags (first byte of every packet)
enum class DbgMsg : uint8_t {
    INIT       = 0x01,   // width(u16) height(u16)
    PIXEL      = 0x02,   // x(u16) y(u16) r g b
    RAY        = 0x03,   // px(u16) py(u16) ox oy oz dx dy dz  (floats)
    HIT        = 0x04,   // px(u16) py(u16) t nx ny nz hitx hity hitz (floats)
    LABEL      = 0x05,   // px(u16) py(u16) len(u16) text...
    FRAME_DONE = 0x06,   // (no payload)
    ROW_DONE   = 0x07,   // y(u16)
};

class DebugVisualizer {
public:
    // Meyer's singleton — constructed on first call, thread-safe in C++11+
    static DebugVisualizer& instance() {
        static DebugVisualizer viz;
        return viz;
    }

    bool connected() const { return m_fd >= 0; }

    // ------------------------------------------------------------------
    // Public API — all safe to call even when viewer is not running
    // ------------------------------------------------------------------

    // Call once after you know the framebuffer dimensions
    void init(int width, int height) {
        if (!connected()) try_connect();
        if (!connected()) return;
        m_width  = width;
        m_height = height;
        uint8_t buf[5];
        buf[0] = static_cast<uint8_t>(DbgMsg::INIT);
        pack_u16(buf + 1, static_cast<uint16_t>(width));
        pack_u16(buf + 3, static_cast<uint16_t>(height));
        send_all(buf, 5);
    }

    // Mirror a pixel write
    void pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (!connected()) return;
        uint8_t buf[8];
        buf[0] = static_cast<uint8_t>(DbgMsg::PIXEL);
        pack_u16(buf + 1, static_cast<uint16_t>(x));
        pack_u16(buf + 3, static_cast<uint16_t>(y));
        buf[5] = r; buf[6] = g; buf[7] = b;
        send_all(buf, 8);
    }

    // Log the ray used for pixel (px,py)
    void ray_info(int px, int py,
                  float ox, float oy, float oz,
                  float dx, float dy, float dz) {
        if (!connected()) return;
        uint8_t buf[1 + 4 + 24];
        buf[0] = static_cast<uint8_t>(DbgMsg::RAY);
        pack_u16(buf + 1, static_cast<uint16_t>(px));
        pack_u16(buf + 3, static_cast<uint16_t>(py));
        pack_f32(buf + 5,  ox); pack_f32(buf + 9,  oy); pack_f32(buf + 13, oz);
        pack_f32(buf + 17, dx); pack_f32(buf + 21, dy); pack_f32(buf + 25, dz);
        send_all(buf, 29);
    }

    // Log a hit for pixel (px,py)
    void hit_info(int px, int py, float t,
                  float nx, float ny, float nz,
                  float hx, float hy, float hz) {
        if (!connected()) return;
        uint8_t buf[1 + 4 + 28];
        buf[0] = static_cast<uint8_t>(DbgMsg::HIT);
        pack_u16(buf + 1, static_cast<uint16_t>(px));
        pack_u16(buf + 3, static_cast<uint16_t>(py));
        pack_f32(buf + 5,  t);
        pack_f32(buf + 9,  nx); pack_f32(buf + 13, ny); pack_f32(buf + 17, nz);
        pack_f32(buf + 21, hx); pack_f32(buf + 25, hy); pack_f32(buf + 29, hz);
        send_all(buf, 33);
    }

    // Attach an arbitrary text label to a pixel
    void label(int px, int py, const std::string& text) {
        if (!connected()) return;
        uint16_t len = static_cast<uint16_t>(std::min(text.size(), size_t(512)));
        uint8_t hdr[7];
        hdr[0] = static_cast<uint8_t>(DbgMsg::LABEL);
        pack_u16(hdr + 1, static_cast<uint16_t>(px));
        pack_u16(hdr + 3, static_cast<uint16_t>(py));
        pack_u16(hdr + 5, len);
        send_all(hdr, 7);
        send_all(reinterpret_cast<const uint8_t*>(text.data()), len);
    }

    // Signal that a full row has been rendered
    void row_done(int y) {
        if (!connected()) return;
        uint8_t buf[3];
        buf[0] = static_cast<uint8_t>(DbgMsg::ROW_DONE);
        pack_u16(buf + 1, static_cast<uint16_t>(y));
        send_all(buf, 3);
    }

    // Signal that the full frame is done
    void frame_done() {
        if (!connected()) return;
        uint8_t buf[1] = { static_cast<uint8_t>(DbgMsg::FRAME_DONE) };
        send_all(buf, 1);
    }

private:
    int m_fd     = -1;
    int m_width  = 0;
    int m_height = 0;

    DebugVisualizer() {
        try_connect();
    }
    ~DebugVisualizer() {
        if (m_fd >= 0) ::close(m_fd);
    }
    DebugVisualizer(const DebugVisualizer&) = delete;
    DebugVisualizer& operator=(const DebugVisualizer&) = delete;

    void try_connect() {
        const char* path = std::getenv("DEBUGVIS_SOCK");
        if (!path) path = "/tmp/debugvis.sock";

        m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (m_fd < 0) return;

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

        if (::connect(m_fd, reinterpret_cast<struct sockaddr*>(&addr),
                      sizeof(addr)) < 0) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    void send_all(const uint8_t* data, size_t len) {
        size_t sent = 0;
        while (sent < len) {
            ssize_t n = ::send(m_fd, data + sent, len - sent, MSG_NOSIGNAL);
            if (n <= 0) {
                // Viewer disconnected — silently disable
                ::close(m_fd);
                m_fd = -1;
                return;
            }
            sent += static_cast<size_t>(n);
        }
    }

    static void pack_u16(uint8_t* dst, uint16_t v) {
        dst[0] = static_cast<uint8_t>(v & 0xFF);
        dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    }
    static void pack_f32(uint8_t* dst, float v) {
        std::memcpy(dst, &v, 4);
    }
};
