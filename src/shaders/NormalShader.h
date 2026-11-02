#pragma once
#include "Shader.h"

class NormalShader : public Shader {
    public:
      color shape_render(const hit_record& rec) const override
      {
        return (rec.normal + vec3(1,1,1)) * .5;
      }
};
