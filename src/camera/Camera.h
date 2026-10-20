#pragma once

#include "../math/ray.h"
#include "../math/vec3.h"

// Minimal abstract base class for cameras
class Camera {
public:
    vec3 position = vec3(0, 0, 0);
    vec3 rotation = vec3(0, 0, 0);

    float vp_width = 2.0f;
    float vp_height = 2.0f;

    int image_width = 100;
    int image_height = 100;

    Camera() = default;

    Camera(int width, int height, float focal_length = 1.0f, float v_height = 2.0f,
           vec3 pos = vec3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : position(pos), rotation(rot),
          vp_height(v_height),
          vp_width(v_height * (float(width) / float(height < 1 ? 1 : height))),
          image_width(width), image_height(height < 1 ? 1 : height) {}

    virtual ~Camera() = default;

    virtual ray shoot_ray(float x, float y) const = 0;
};

using camera = Camera;
