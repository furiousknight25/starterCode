#pragma once

#include "Shader.h"
#include "Light.h"
#include <algorithm>

class LambertianShader : public Shader {
    public:
        color albedo;

        LambertianShader(const color& albedo) : albedo(albedo) {}
        
        color shape_render(const hit_record& rec) const override {
            if (rec.normal.length() < 1e-6f) {
                return Shader::shape_render(rec);
            }

            vec3 normal = normalize(rec.normal);

            color sum(0.0f, 0.0f, 0.0f);
           
            if (rec.lights) {
              for (const auto &light : *rec.lights) {
                
                vec3 light_dir = normalize(light.position - rec.p);
                
                float dp = dot(light_dir, normal);
                if (dp > 0.0f) {
                    vec3 col = light.intensity * dp * albedo * light.color;
                    // std::cout << light_dir << " " << normal;
                    sum += col;
                }
              }
            }

            return sum;
        }
};
