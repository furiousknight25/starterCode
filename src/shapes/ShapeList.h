#pragma once

#include "shape.h"
#include "shaders/Light.h"
#include "BVHNode.h"
#include <vector>
#include <memory>

//I only want a header, I will assume that there only ever will be one shapelist at a time if a shapelist does exist

class ShapeList : public Shape {
public:
    std::vector<std::shared_ptr<Shape>> objects;
    std::vector<Light> lights;

    // Optional BVH root for accelerated queries (decoupled: falls back to linear search if null)
    std::shared_ptr<Shape> bvh_root{nullptr};

    ShapeList() = default;
    ShapeList(std::shared_ptr<Shape> object) { add(object); }

    void clear() { objects.clear(); bvh_root = nullptr; }
    void add(std::shared_ptr<Shape> object) { objects.push_back(object); bvh_root = nullptr; }
    void add_light(const Light& light) {lights.push_back(light);} // how do you have a const reference?

    // Build or rebuild the BVH acceleration tree from objects
    void build_bvh() {
        if (!objects.empty()) {
            bvh_root = std::make_shared<BVHNode>(objects);
        } else {
            bvh_root = nullptr;
        }
    }

    void clear_bvh() {
        bvh_root = nullptr;
    }

    bool intersect(const ray& r) const override { //basic multi, hard code buffer
        if (bvh_root) {
            return bvh_root->intersect(r);
        }
        for (const auto& object : objects) {
            if (object->intersect(r)) {
                return true;
            }
        }
        return false;
    }

    bool intersect(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;

        if (bvh_root) {
            hit_anything = bvh_root->intersect(r, t_min, t_max, rec);
        } else {
            auto closest_so_far = t_max; //closest t value?
            for (const auto& object : objects) {
                if (object->intersect(r, t_min, closest_so_far, temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;

                    rec = temp_rec;
                }
            }
        }

        if (hit_anything) {
          rec.filtered_lights.clear();
          for (int i = 0; i < lights.size(); ++i) {
             
            ray blockr(rec.p + (0.001f * rec.normal), normalize(lights.at(i).position - rec.p));
            double light_dist = (lights.at(i).position - rec.p).length();
            
            bool in_shadow = false;
            if (bvh_root) {
                hit_record shadow_rec;
                in_shadow = bvh_root->intersect(blockr, 0.001, light_dist, shadow_rec);
            } else {
                for (const auto &obj : objects) {
                    hit_record shadow_rec;
                    in_shadow = obj->intersect(blockr, 0.001, light_dist, shadow_rec);

                    if (in_shadow == true) {
                        break;
                    }
                }
            }
            if (!in_shadow) {
                rec.filtered_lights.push_back(lights.at(i));
            }
          }
          rec.lights = &rec.filtered_lights;
        }

        return hit_anything;
    }//hit rec in the future have access to the shape list?
    bool intersect(const ray& r, interval ray_t, hit_record& rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }

    AABB bounding_box() const override {
        if (bvh_root) return bvh_root->bounding_box();
        AABB box = AABB::empty;
        for (const auto& obj : objects) {
            box = AABB(box, obj->bounding_box());
        }
        return box;
    }

};

using Scene = ShapeList;
using shape_list = ShapeList;
