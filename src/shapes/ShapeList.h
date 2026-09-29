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
};

using Scene = ShapeList;
using shape_list = ShapeList;
