#include "ShapeList.h"
#include <limits>

bool ShapeList::intersect(const ray& r) const {
    hit_record temp_rec;
    return intersect(r, 0.001, std::numeric_limits<double>::infinity(), temp_rec);
}

bool ShapeList::intersect(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->intersect(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}
