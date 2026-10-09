#pragma once

#include "shape.h"
#include <vector>
#include <memory>

class ShapeList : public Shape {
public:
    std::vector<std::shared_ptr<Shape>> objects;

    ShapeList() = default;
    ShapeList(std::shared_ptr<Shape> object) { add(object); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Shape> object) { objects.push_back(object); }
    size_t size() const { return objects.size(); }

    bool intersect(const ray& r) const override;
    bool intersect(const ray& r) override {
        return static_cast<const ShapeList*>(this)->intersect(r);
    }

    bool intersect(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    bool intersect(const ray& r, interval ray_t, hit_record& rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }

    // Point Light in the Scene (default from lab instructions: (0, 10, 5))
    point3 light_pos{0.0, 10.0, 5.0};
    const point3& get_light_position() const { return light_pos; }
    void set_light_position(const point3& p) { light_pos = p; }

    // Compute ray color for closest shape, using its shader
    color computeRayColor(const ray& r, const color& bg_color = color(0.2, 0.2, 0.2)) const;
};

using Scene = ShapeList;
using shape_list = ShapeList;
