#pragma once

#include "../math/vec3.h"
#include "../math/ray.h"
#include "../math/interval.h"
#include <algorithm>
#include <cmath>

using point3 = vec3;

// ============================================================================
// Axis-Aligned Bounding Box (AABB)
// Represents a 3D rectangular box aligned with the coordinate axes.
// ============================================================================
class AABB {
public:
    interval x;
    interval y;
    interval z;

    // Default constructor: creates an empty bounding box
    AABB() = default;

    // Construct from 3 intervals along x, y, and z axes
    AABB(const interval& x, const interval& y, const interval& z)
        : x(x), y(y), z(z) {
        pad_to_minimums();
    }

    // Construct from two opposing corner points
    AABB(const vec3& a, const vec3& b) {
        x = (a.x() <= b.x()) ? interval(a.x(), b.x()) : interval(b.x(), a.x());
        y = (a.y() <= b.y()) ? interval(a.y(), b.y()) : interval(b.y(), a.y());
        z = (a.z() <= b.z()) ? interval(a.z(), b.z()) : interval(b.z(), a.z());
        pad_to_minimums();
    }

    // Construct an AABB that encloses two existing AABBs
    AABB(const AABB& box0, const AABB& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }

    // Returns the interval for axis index: 0 = x, 1 = y, 2 = z
    const interval& axis_interval(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    vec3 min() const { return vec3(x.min, y.min, z.min); }
    vec3 max() const { return vec3(x.max, y.max, z.max); }
    vec3 centroid() const { return (min() + max()) * 0.5f; }

    // Returns the index (0=x, 1=y, 2=z) of the axis with the largest extent
    int longest_axis() const {
        double x_size = x.size();
        double y_size = y.size();
        double z_size = z.size();
        if (x_size > y_size)
            return (x_size > z_size) ? 0 : 2;
        else
            return (y_size > z_size) ? 1 : 2;
    }

    // Fast ray-AABB slab intersection test
    bool intersect(const ray& r, double t_min, double t_max) const {
        const vec3& ray_orig = r.origin();
        const vec3& ray_dir  = r.direction();

        for (int axis = 0; axis < 3; axis++) {
            const interval& ax = axis_interval(axis);
            double dir = ray_dir[axis];

            if (std::abs(dir) < 1e-12) {
                // Ray is parallel to the slab; check if origin is inside this slab
                if (ray_orig[axis] < ax.min || ray_orig[axis] > ax.max)
                    return false;
                continue;
            }

            const double adinv = 1.0 / dir;
            double t0 = (ax.min - ray_orig[axis]) * adinv;
            double t1 = (ax.max - ray_orig[axis]) * adinv;

            if (t0 > t1) std::swap(t0, t1);

            if (t0 > t_min) t_min = t0;
            if (t1 < t_max) t_max = t1;

            if (t_max <= t_min)
                return false;
        }
        return true;
    }

    bool intersect(const ray& r, interval ray_t) const {
        return intersect(r, ray_t.min, ray_t.max);
    }

    static const AABB empty;
    static const AABB universe;

private:
    // Ensures none of the dimensions have zero thickness (prevents issues with flat shapes)
    void pad_to_minimums() {
        constexpr double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

inline const AABB AABB::empty(interval::empty, interval::empty, interval::empty);
inline const AABB AABB::universe(interval::universe, interval::universe, interval::universe);

using aabb = AABB;
