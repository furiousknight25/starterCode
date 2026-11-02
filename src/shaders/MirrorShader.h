#pragma once

#include "Shader.h"
#include <algorithm>

class MirrorShader: public Shader {
    public:
        MirrorShader() {}
        
        bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const override {
            vec3 normal = normalize(rec.normal);

            color sum(0.0f, 0.0f, 0.0f); 
            
            vec3 reflected_v = r.direction() - 2 * (dot(r.direction(), normal) * normal); //does r.direction() really have to be normalized??
            
            scattered = r.spawn_bounce(rec.p + 0.001f * normal, reflected_v);
            
            attenuation = vec3::ONE; //fully reflective
            
            return true;
        }
};
