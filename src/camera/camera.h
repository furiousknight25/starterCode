#pragma once

#include "../math/ray.h"
#include "../math/vec3.h"

// Abstract base class for cameras
class camera {
public:
    float aspect_ratio = 16.0f / 9.0f;
    int image_width = 400;
    int image_height = 225;

    // Viewport dimensions in world space
    float viewport_height = 2.0f;
    float viewport_width = 2.0f * (16.0f / 9.0f);

    // Camera position / center in world space
    point3 camera_center = point3(0, 0, 0);
    point3& position = camera_center;

    // Camera rotation (Euler angles in degrees: pitch (x), yaw (y), roll (z))
    vec3 rotation = vec3(0, 0, 0);

    camera() = default;

    camera(int width, float aspect_ratio = 16.0f / 9.0f, float v_height = 2.0f,
           point3 center = point3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : aspect_ratio(aspect_ratio), image_width(width), viewport_height(v_height),
          camera_center(center), rotation(rot) {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        viewport_width = viewport_height * (float(image_width) / image_height);
    }

    camera(int width, int height, float v_height = 2.0f,
           point3 center = point3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : image_width(width),
          image_height(height < 1 ? 1 : height),
          aspect_ratio(float(width) / (height < 1 ? 1 : height)),
          viewport_height(v_height),
          camera_center(center), rotation(rot) {
        viewport_width = viewport_height * (float(image_width) / image_height);
    }

    camera(const camera& other)
        : aspect_ratio(other.aspect_ratio),
          image_width(other.image_width),
          image_height(other.image_height),
          viewport_height(other.viewport_height),
          viewport_width(other.viewport_width),
          camera_center(other.camera_center),
          rotation(other.rotation) {}

    camera& operator=(const camera& other) {
        if (this != &other) {
            aspect_ratio = other.aspect_ratio;
            image_width = other.image_width;
            image_height = other.image_height;
            viewport_height = other.viewport_height;
            viewport_width = other.viewport_width;
            camera_center = other.camera_center;
            rotation = other.rotation;
        }
        return *this;
    }

    camera(camera&& other) noexcept
        : aspect_ratio(other.aspect_ratio),
          image_width(other.image_width),
          image_height(other.image_height),
          viewport_height(other.viewport_height),
          viewport_width(other.viewport_width),
          camera_center(other.camera_center),
          rotation(other.rotation) {}

    camera& operator=(camera&& other) noexcept {
        if (this != &other) {
            aspect_ratio = other.aspect_ratio;
            image_width = other.image_width;
            image_height = other.image_height;
            viewport_height = other.viewport_height;
            viewport_width = other.viewport_width;
            camera_center = other.camera_center;
            rotation = other.rotation;
        }
        return *this;
    }

    virtual ~camera() = default;

    virtual void initialize() {}

    // Position mutator / accessor
    virtual void set_position(const point3& pos) {
        camera_center = pos;
        initialize();
    }
    const point3& get_position() const { return camera_center; }

    // Rotation mutators / accessor (angles in degrees: pitch (x), yaw (y), roll (z))
    virtual void set_rotation(const vec3& rot) {
        rotation = rot;
        initialize();
    }
    virtual void set_rotation(float pitch, float yaw, float roll) {
        rotation = vec3(pitch, yaw, roll);
        initialize();
    }
    const vec3& get_rotation() const { return rotation; }

    // Pure virtual method to generate a ray passing through pixel (i, j)
    virtual ray generateRay(int i, int j) = 0;
    virtual ray generateRay(int i, int j) const = 0;
};
