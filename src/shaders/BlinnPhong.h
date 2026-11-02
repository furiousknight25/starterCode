#pragma once

#include "Shader.h"
#include "Light.h"
#include <algorithm>

class BlinnPhong: public Shader {
    public:
        color albedo;
        float spec_intensity = 2.0f;

        BlinnPhong(const color& albedo, float spec_intensity) : albedo(albedo), spec_intensity(spec_intensity) {}
        
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
                    sum += col;
                    
                    vec3 half_vector = normalize((light_dir + rec.view_dir) / 2);
                    vec3 spec_col = pow(dot(half_vector, rec.normal), spec_intensity) * vec3(1.0,1.0,1.0);
                    sum += spec_col;
                }

                
              }
            }

            return sum;
        }
};
