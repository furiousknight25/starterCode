#pragma once

#include "shape.h"
#include "shaders/Shader.h"
#include "../math/vec3.h"
#include <vector>
#include <cmath>

class Triangle : public Shape {
public:
    Triangle() = default;

    // 3-vertex constructor (used by SceneLoader and unit tests)
    Triangle(const vec3& v0, const vec3& v1, const vec3& v2, std::shared_ptr<Shader> shader = nullptr)
        : Shape(v0, vec3::ZERO, color(0.8, 0.8, 0.8), shader), points{v0, v1, v2} {}

    // Vector constructor
    Triangle(const vec3 &center, const vec3 &rotation, const std::vector<vec3>& points, std::shared_ptr<Shader> shader = nullptr)
        : Shape(center, rotation, color(0.8, 0.8, 0.8), shader), points(points) {}

    // Vertex accessors
    const vec3& A() const { return points[0]; }
    const vec3& B() const { return points[1]; }
    const vec3& C() const { return points[2]; }
    const vec3& vertex(size_t i) const { return points[i]; }

    vec3 normal() const {
        if (points.size() < 3) return vec3(0.0, 0.0, 1.0);
        vec3 e1 = points[1] - points[0];
        vec3 e2 = points[2] - points[0];
        return normalize(cross(e1, e2));
    }

    bool intersect(const ray& r) const override {
        hit_record rec;
        return intersect(r, 0.001, 1e9, rec);
    }

    bool intersect(const ray& r, double t_min, double t_max, hit_record &rec) const override {
        if (points.size() < 3) return false;

        const vec3& A = points[0];
        const vec3& B = points[1];
        const vec3& C = points[2];

        vec3 e1 = B - A;
        vec3 e2 = C - A;
        
        float denom = dot(e1, cross(r.direction(), e2));
        
        if (std::abs(denom) < 1e-6f) { return false; }

        float inv_denom = 1.0f / denom;
        float t = (dot(r.origin() - A, cross(e1, e2))) * inv_denom;
        float beta = (dot(A - r.origin(), cross(e2, r.direction()))) * inv_denom;
        float gamma = (dot(A - r.origin(), cross(r.direction(), e1))) * inv_denom;
        float alpha = 1.0f - beta - gamma;

        if (!(t >= 0 && alpha >= 0 && beta >= 0 && gamma >= 0 && t > t_min && t < t_max)) {
            return false;
        }
        
        vec3 outward_normal = normalize(cross(e1, e2));
        
        rec.set_face_normal(r, outward_normal);
        rec.t = t;
        rec.p = r.origin() + t * r.direction();
        rec.shader = shader;
        rec.view_dir = normalize(-r.direction());
        
        return true;
    }

    bool intersect(const ray& r, interval ray_t, hit_record &rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }

    AABB bounding_box() const override {
        if (points.empty()) return AABB::empty;
        vec3 min_p = points[0];
        vec3 max_p = points[0];
        for (size_t i = 1; i < points.size(); ++i) {
            min_p = vec3(std::min(min_p.x(), points[i].x()),
                         std::min(min_p.y(), points[i].y()),
                         std::min(min_p.z(), points[i].z()));
            max_p = vec3(std::max(max_p.x(), points[i].x()),
                         std::max(max_p.y(), points[i].y()),
                         std::max(max_p.z(), points[i].z()));
        }
        return AABB(min_p, max_p);
    }

private:
    std::vector<vec3> points;
};
