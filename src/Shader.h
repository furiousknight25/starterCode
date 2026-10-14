#pragma once

#include "math/vec3.h"
#include "shapes/hit_record.h"
#include <algorithm>
#include <cmath>
#include <memory>

// simple point light (hardcoded to (0, 10, 5) from the lab slides)
struct PointLight {
    point3 position{0.0, 10.0, 5.0};
    color intensity{1.0, 1.0, 1.0};

    PointLight() = default;
    PointLight(const point3& pos, const color& col = color(1.0, 1.0, 1.0))
        : position(pos), intensity(col) {}
};

// base abstract shader
class Shader {
public:
    virtual ~Shader() = default;
    virtual color rayColor(const HitStructure &h) = 0;
};

// normal shader (just for visualizing surface normals)
class NormalShader : public Shader {
public:
    color rayColor(const HitStructure &h) override {
        vec3 n = unit_vector(h.normal);
        return 0.5 * (n + color(1.0, 1.0, 1.0));
    }
};

// Lambertian diffuse (matte) shader
class Lambertian : public Shader {
public:
    color diffuse_color{0.8, 0.8, 0.8};
    point3 light_pos{0.0, 10.0, 5.0};

    Lambertian() = default;
    Lambertian(const color& c, const point3& lpos = point3(0, 10, 5))
        : diffuse_color(c), light_pos(lpos) {}

    color rayColor(const HitStructure &h) override {
        vec3 normal = unit_vector(h.normal);
        vec3 lightDir = unit_vector(light_pos - h.p);

        // Surface is only lit if normal points towards the light (dot > 0)
        double nDotl = std::max(0.0, dot(normal, lightDir));

        return diffuse_color * nDotl;
    }
};

// Blinn-Phong specular (shiny) shader
class BlinnPhong : public Shader {
public:
    color diffuse_color{0.8, 0.8, 0.8};
    color specular_color{1.0, 1.0, 1.0}; // white specular highlight
    double phongExp{32.0};
    point3 light_pos{0.0, 10.0, 5.0};

    BlinnPhong() = default;
    BlinnPhong(const color& c, double exp = 32.0, const color& spec = color(1.0, 1.0, 1.0), const point3& lpos = point3(0, 10, 5))
        : diffuse_color(c), specular_color(spec), phongExp(exp), light_pos(lpos) {}

    color rayColor(const HitStructure &h) override {
        vec3 normal = unit_vector(h.normal);
        vec3 lightDir = unit_vector(light_pos - h.p);
        vec3 viewDir = unit_vector(h.view_dir);

        // 1. Diffuse part (Lambertian)
        double nDotl = std::max(0.0, dot(normal, lightDir));
        color diffuseComponent = diffuse_color * nDotl;

        // 2. Specular part using half-vector (bisector between light and view directions)
        vec3 halfVector = unit_vector(lightDir + viewDir);

        double spec = 0.0;
        if (nDotl > 0.0) {
            spec = std::pow(std::max(0.0, dot(normal, halfVector)), phongExp);
        }

        color specularComponent = specular_color * spec;

        return diffuseComponent + specularComponent;
    }
};
