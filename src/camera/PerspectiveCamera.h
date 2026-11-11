#pragma once

#include "Camera.h"
#include "../Framebuffer.h"
#include <cmath>

// Perspective camera with pinhole projection
class PerspectiveCamera : public Camera {
public:
    float focal_length = 1.0f;
    vec3 forward = vec3(0, 0, -1);
    vec3 u = vec3(1, 0, 0); // camera right
    vec3 v = vec3(0, 1, 0); // camera up

    // Constructor 1: Direct dimensions (e.g. PerspectiveCamera(100, 100, 1.0))
    PerspectiveCamera(int width = 100, int height = 100, float focal_length = 1.0f,
                      float v_height = 2.0f,
                      vec3 pos = vec3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : Camera(width, height, focal_length, v_height, pos, rot), focal_length(focal_length) {
        set_view_dir(vec3(0, 0, -1));
    }

    void set_view_dir(const vec3& dir, const vec3& up = vec3(0, 1, 0)) {
        if (dir.length_squared() < 1e-8f) {
            forward = vec3(0, 0, -1);
        } else {
            forward = normalize(dir);
        }
        vec3 world_up = up;
        if (std::abs(dot(forward, world_up)) > 0.999f) {
            world_up = vec3(0, 0, 1);
            if (std::abs(dot(forward, world_up)) > 0.999f) {
                world_up = vec3(1, 0, 0);
            }
        }
        u = normalize(cross(forward, world_up));
        v = normalize(cross(u, forward));
    }

    void set_lookat(const vec3& target, const vec3& up = vec3(0, 1, 0)) {
        set_view_dir(target - position, up);
    }

    void set_viewport(float plane_width, int width, int height) {
        image_width = width;
        image_height = (height < 1) ? 1 : height;
        image_plane_width = plane_width;
        vp_width = plane_width;
        vp_height = plane_width * (float(image_height) / float(image_width));
    }

    // Shoots a ray through pixel (x, y)
    ray shoot_ray(float x, float y) const override {
        float du = vp_width / float(image_width);
        float dv = vp_height / float(image_height);

        // Center on pixel (x + 0.5, y + 0.5)
        float px = (x + 0.5f) * du - (vp_width * 0.5f);
        float py = (vp_height * 0.5f) - (y + 0.5f) * dv;

        vec3 ray_dir = px * u + py * v + focal_length * forward;

        // Rotate if camera has rotation
        if (rotation.x() != 0.0f || rotation.y() != 0.0f || rotation.z() != 0.0f) {
            constexpr float deg2rad = 3.14159265358979323846f / 180.0f;
            float pitch = rotation.x() * deg2rad;
            float yaw   = rotation.y() * deg2rad;
            float roll  = rotation.z() * deg2rad;

            float y1 = ray_dir.y() * std::cos(pitch) - ray_dir.z() * std::sin(pitch);
            float z1 = ray_dir.y() * std::sin(pitch) + ray_dir.z() * std::cos(pitch);

            float x2 =  ray_dir.x() * std::cos(yaw) + z1 * std::sin(yaw);
            float z2 = -ray_dir.x() * std::sin(yaw) + z1 * std::cos(yaw);

            float x3 = x2 * std::cos(roll) - y1 * std::sin(roll);
            float y3 = x2 * std::sin(roll) + y1 * std::cos(roll);

            ray_dir = vec3(x3, y3, z2);
        }

        return ray(position, normalize(ray_dir));
    }
};

