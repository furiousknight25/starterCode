#pragma once

#include "camera.h"
#include <cmath>
#include <algorithm>

// Perspective camera with pinhole projection extending the base abstract camera class
class perspective_camera : public camera {
public:
    float focal_length = 1.0f;

    // Orthonormal basis vectors
    vec3 u = vec3(1, 0, 0); // camera right
    vec3 v = vec3(0, 1, 0); // camera up
    vec3 w = vec3(0, 0, 1); // camera backward (opposite to forward look direction)

    vec3 viewport_u;
    vec3 viewport_v;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    point3 pixel00_loc;
    vec3 pixel00_offset;

    perspective_camera() {
        initialize();
    }

    perspective_camera(int width, float aspect_ratio = 16.0f / 9.0f, float focal_length = 1.0f,
                       float v_height = 2.0f, point3 center = point3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : camera(width, aspect_ratio, v_height, center, rot), focal_length(focal_length) {
        initialize();
    }

    perspective_camera(int width, int height, float focal_length = 1.0f,
                       float v_height = 2.0f, point3 center = point3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : camera(width, height, v_height, center, rot), focal_length(focal_length) {
        initialize();
    }

    // Overload allowing position and rotation without specifying focal_length and v_height
    perspective_camera(int width, float aspect_ratio, const point3& center,
                       const vec3& rot = vec3(0, 0, 0), float focal_length = 1.0f, float v_height = 2.0f)
        : camera(width, aspect_ratio, v_height, center, rot), focal_length(focal_length) {
        initialize();
    }

    perspective_camera(const perspective_camera& other)
        : camera(other),
          focal_length(other.focal_length),
          u(other.u), v(other.v), w(other.w),
          viewport_u(other.viewport_u), viewport_v(other.viewport_v),
          pixel_delta_u(other.pixel_delta_u), pixel_delta_v(other.pixel_delta_v),
          pixel00_loc(other.pixel00_loc), pixel00_offset(other.pixel00_offset) {}

    perspective_camera& operator=(const perspective_camera& other) {
        if (this != &other) {
            camera::operator=(other);
            focal_length = other.focal_length;
            u = other.u;
            v = other.v;
            w = other.w;
            viewport_u = other.viewport_u;
            viewport_v = other.viewport_v;
            pixel_delta_u = other.pixel_delta_u;
            pixel_delta_v = other.pixel_delta_v;
            pixel00_loc = other.pixel00_loc;
            pixel00_offset = other.pixel00_offset;
        }
        return *this;
    }

    perspective_camera(perspective_camera&& other) noexcept
        : camera(std::move(other)),
          focal_length(other.focal_length),
          u(other.u), v(other.v), w(other.w),
          viewport_u(other.viewport_u), viewport_v(other.viewport_v),
          pixel_delta_u(other.pixel_delta_u), pixel_delta_v(other.pixel_delta_v),
          pixel00_loc(other.pixel00_loc), pixel00_offset(other.pixel00_offset) {}

    perspective_camera& operator=(perspective_camera&& other) noexcept {
        if (this != &other) {
            camera::operator=(std::move(other));
            focal_length = other.focal_length;
            u = other.u;
            v = other.v;
            w = other.w;
            viewport_u = other.viewport_u;
            viewport_v = other.viewport_v;
            pixel_delta_u = other.pixel_delta_u;
            pixel_delta_v = other.pixel_delta_v;
            pixel00_loc = other.pixel00_loc;
            pixel00_offset = other.pixel00_offset;
        }
        return *this;
    }

    // Computes basis vectors u, v, w from Euler angles (pitch, yaw, roll in degrees)
    void update_basis_from_rotation() {
        constexpr float pi = 3.14159265358979323846f;
        float pitch_rad = static_cast<float>(rotation.x()) * pi / 180.0f;
        float yaw_rad   = static_cast<float>(rotation.y()) * pi / 180.0f;
        float roll_rad  = static_cast<float>(rotation.z()) * pi / 180.0f;

        float cp = std::cos(pitch_rad);
        float sp = std::sin(pitch_rad);
        float cy = std::cos(yaw_rad);
        float sy = std::sin(yaw_rad);
        float cr = std::cos(roll_rad);
        float sr = std::sin(roll_rad);

        // Backward vector w (-forward):
        // Default forward is (0, 0, -1)
        // With yaw (positive = turn right towards +x) and pitch (positive = look up towards +y):
        // forward = (sy * cp, sp, -cy * cp)
        // w = -forward = (-sy * cp, -sp, cy * cp)
        w = vec3(-sy * cp, -sp, cy * cp);

        // Unrolled camera right u0 and camera up v0:
        vec3 u0 = vec3(cy, 0.0f, sy);
        vec3 v0 = vec3(-sy * sp, cp, cy * sp);

        // Apply roll around w:
        // positive roll rotates clockwise (right vector towards up vector)
        u = cr * u0 + sr * v0;
        v = -sr * u0 + cr * v0;
    }

    void initialize_viewport() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        viewport_width = viewport_height * (float(image_width) / float(image_height));

        // Vectors across horizontal and down vertical viewport edges
        viewport_u = viewport_width * u;
        viewport_v = -viewport_height * v;

        // Horizontal and vertical delta vectors from pixel to pixel
        pixel_delta_u = viewport_u / float(image_width);
        pixel_delta_v = viewport_v / float(image_height);

        // Viewport offset from camera_center to upper left pixel
        pixel00_offset = -focal_length * w
                       - viewport_u / 2.0f
                       - viewport_v / 2.0f
                       + 0.5f * (pixel_delta_u + pixel_delta_v);

        pixel00_loc = camera_center + pixel00_offset;
    }

    void initialize() override {
        update_basis_from_rotation();
        initialize_viewport();
    }

    void set_position(const point3& pos) override {
        camera_center = pos;
        pixel00_loc = camera_center + pixel00_offset;
    }

    void set_rotation(const vec3& rot) override {
        rotation = rot;
        initialize();
    }

    void set_rotation(float pitch, float yaw, float roll) override {
        rotation = vec3(pitch, yaw, roll);
        initialize();
    }

    // Orient camera towards target point from current position
    void set_lookat(const point3& target, const vec3& vup = vec3(0, 1, 0)) {
        vec3 forward = unit_vector(target - camera_center);
        w = -forward;
        vec3 right_trial = cross(vup, w);
        if (right_trial.length_squared() < 1e-8f) {
            right_trial = cross(vec3(0, 0, 1), w);
            if (right_trial.length_squared() < 1e-8f) {
                right_trial = cross(vec3(1, 0, 0), w);
            }
        }
        u = unit_vector(right_trial);
        v = cross(w, u);

        // Synchronize rotation Euler angles
        constexpr float pi = 3.14159265358979323846f;
        float pitch = std::asin(std::clamp(static_cast<float>(forward.y()), -1.0f, 1.0f)) * 180.0f / pi;
        float yaw = std::atan2(static_cast<float>(forward.x()), static_cast<float>(-forward.z())) * 180.0f / pi;
        rotation = vec3(pitch, yaw, 0.0f);

        initialize_viewport();
    }

    // Orient camera from a specified position towards target point
    void look_at(const point3& from, const point3& target, const vec3& vup = vec3(0, 1, 0)) {
        camera_center = from;
        set_lookat(target, vup);
    }

    static perspective_camera create_lookat(int width, float aspect_ratio, const point3& lookfrom,
                                           const point3& lookat, const vec3& vup = vec3(0, 1, 0),
                                           float focal_length = 1.0f, float v_height = 2.0f) {
        perspective_camera cam(width, aspect_ratio, focal_length, v_height, lookfrom);
        cam.set_lookat(lookat, vup);
        return cam;
    }

    ray generateRay(int i, int j) override {
        vec3 ray_direction = pixel00_offset + (float(i) * pixel_delta_u) + (float(j) * pixel_delta_v);
        return ray(camera_center, ray_direction);
    }

    ray generateRay(int i, int j) const override {
        vec3 ray_direction = pixel00_offset + (float(i) * pixel_delta_u) + (float(j) * pixel_delta_v);
        return ray(camera_center, ray_direction);
    }

    ray generateRay(float u_pixel, float v_pixel) override {
        vec3 ray_direction = pixel00_offset + (u_pixel * pixel_delta_u) + (v_pixel * pixel_delta_v);
        return ray(camera_center, ray_direction);
    }

    ray generateRay(float u_pixel, float v_pixel) const override {
        vec3 ray_direction = pixel00_offset + (u_pixel * pixel_delta_u) + (v_pixel * pixel_delta_v);
        return ray(camera_center, ray_direction);
    }
};

// Type aliases for naming flexibility
using PerspectiveCamera = perspective_camera;
using perspectiveCamera = perspective_camera;
