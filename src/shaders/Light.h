#pragma once

#include "math/vec3.h"

class Light {
    public:
        Light(
            const vec3& position,
            const vec3& col = vec3(1.0f, 1.0f, 1.0f),
            float intensity = 1.0f
        ) : position(position), color(col), intensity(intensity) {}
        
        vec3 position;
        vec3 color;
        float intensity;
};