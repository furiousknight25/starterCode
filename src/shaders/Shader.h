#pragma once

#include "math/vec3.h"
#include "shapes/hit_record.h"
#include <algorithm>
#include <cmath>
#include <memory>

// base abstract shader
class Shader {
public:
    virtual ~Shader() = default; //deconstructer, what is with the default thing?
    
    virtual color shape_render(const hit_record& rec) const {
        return vec3(0.0f,0.0f,0.0f);
    }

    virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const {
        return false;
    }

};
