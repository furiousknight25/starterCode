#pragma once

#include "shape.h"
#include "AABB.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

// ============================================================================
// BVHNode (Bounding Volume Hierarchy Node)
//
// A spatial acceleration structure that organizes shapes into a binary tree of
// Axis-Aligned Bounding Boxes (AABB). Reduces ray-object intersection complexity
// from O(N) to O(log N).
//
// Decoupled design:
// - BVHNode is just another Shape (implements Shape interface).
// - To remove or rewrite BVHNode, simply modify or replace this file.
// ============================================================================
class BVHNode : public Shape {
public:
    std::shared_ptr<Shape> left{nullptr};
    std::shared_ptr<Shape> right{nullptr};
    AABB bbox;

    BVHNode() = default;

    // Construct a BVH tree from a list of shapes
    explicit BVHNode(const std::vector<std::shared_ptr<Shape>>& src_objects) {
        if (src_objects.empty()) {
            bbox = AABB::empty;
            return;
        }
        std::vector<std::shared_ptr<Shape>> objects = src_objects;
        build(objects, 0, objects.size());
    }

    // Construct a BVH subtree spanning objects[start .. end)
    BVHNode(std::vector<std::shared_ptr<Shape>>& objects, size_t start, size_t end) {
        build(objects, start, end);
    }

    // Recursive BVH construction
    void build(std::vector<std::shared_ptr<Shape>>& objects, size_t start, size_t end) {
        size_t object_span = end - start;
        if (object_span == 0) {
            bbox = AABB::empty;
            return;
        }

        // 1. Compute the enclosing bounding box for all shapes in this span
        bbox = AABB::empty;
        for (size_t i = start; i < end; ++i) {
            bbox = AABB(bbox, objects[i]->bounding_box());
        }

        // 2. Select the axis with the largest spatial spread for splitting
        int axis = bbox.longest_axis();

        auto comparator = [axis](const std::shared_ptr<Shape>& a, const std::shared_ptr<Shape>& b) {
            return a->bounding_box().axis_interval(axis).min < b->bounding_box().axis_interval(axis).min;
        };

        // 3. Base cases and recursive split
        if (object_span == 1) {
            left = right = objects[start];
        } else if (object_span == 2) {
            if (comparator(objects[start], objects[start + 1])) {
                left = objects[start];
                right = objects[start + 1];
            } else {
                left = objects[start + 1];
                right = objects[start];
            }
        } else {
            // Sort shapes along the chosen split axis
            std::sort(objects.begin() + start, objects.begin() + end, comparator);

            size_t mid = start + object_span / 2;
            left = std::make_shared<BVHNode>(objects, start, mid);
            right = std::make_shared<BVHNode>(objects, mid, end);
        }

        bbox = AABB(left->bounding_box(), right->bounding_box());
    }

    // Quick boolean intersection (used for shadow rays and visibility checks)
    bool intersect(const ray& r) const override {
        if (!left) return false;
        if (!bbox.intersect(r, 0.001, std::numeric_limits<double>::infinity()))
            return false;

        if (left == right)
            return left->intersect(r);

        return left->intersect(r) || right->intersect(r);
    }

    // Full range-bounded intersection query returning the closest hit
    bool intersect(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        if (!left) return false;

        // Early rejection: if ray misses the bounding box, skip children
        if (!bbox.intersect(r, t_min, t_max))
            return false;

        // Leaf node with a single shape
        if (left == right)
            return left->intersect(r, t_min, t_max, rec);

        // Test left subtree
        bool hit_left = left->intersect(r, t_min, t_max, rec);

        // Test right subtree; if left hit, only consider hits closer than rec.t
        bool hit_right = right->intersect(r, t_min, hit_left ? rec.t : t_max, rec);

        return hit_left || hit_right;
    }

    bool intersect(const ray& r, interval ray_t, hit_record& rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }

    AABB bounding_box() const override {
        return bbox;
    }
};

using bvh_node = BVHNode;
