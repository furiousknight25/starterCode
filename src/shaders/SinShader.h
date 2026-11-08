#pragma once
#include "Shader.h"
#include <cmath>

class SinShader : public Shader {
    public:
      color shape_render(const hit_record& rec) const override
      {
        float sin_val = (std::sin(rec.p.x() * 10) + 1.0f) * 0.5f;
        
        float sin_val_y = (std::sin(rec.p.y() * 5) + 1.0f) * 0.5f;

        return vec3(sin_val, sin_val, sin_val_y);
      }
};
