#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "shapes/AABB.h"
#include "shapes/BVHNode.h"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"
#include "shapes/ShapeList.h"

constexpr double EPSILON = 1.0e-4;

TEST_CASE("AABB Construction and Properties", "[bvh][aabb]") {
    SECTION("Construct from min and max points") {
        vec3 p0(-1.0f, -2.0f, -3.0f);
        vec3 p1(4.0f, 5.0f, 6.0f);
        AABB box(p0, p1);

        CHECK(box.x.min == -1.0);
        CHECK(box.x.max == 4.0);
        CHECK(box.y.min == -2.0);
        CHECK(box.y.max == 5.0);
        CHECK(box.z.min == -3.0);
        CHECK(box.z.max == 6.0);

        CHECK(box.longest_axis() == 2); // z size is 9, x is 5, y is 7
    }

    SECTION("Construct with inverted points automatically orders extrema") {
        vec3 p0(10.0f, 20.0f, 30.0f);
        vec3 p1(-5.0f, -10.0f, -15.0f);
        AABB box(p0, p1);

        CHECK(box.x.min == -5.0);
        CHECK(box.x.max == 10.0);
        CHECK(box.y.min == -10.0);
        CHECK(box.y.max == 20.0);
        CHECK(box.z.min == -15.0);
        CHECK(box.z.max == 30.0);
    }

    SECTION("Union of two bounding boxes") {
        AABB box1(vec3(-1, -1, -1), vec3(1, 1, 1));
        AABB box2(vec3(2, 2, 2), vec3(4, 5, 6));
        AABB united(box1, box2);

        CHECK(united.x.min == -1.0);
        CHECK(united.x.max == 4.0);
        CHECK(united.y.min == -1.0);
        CHECK(united.y.max == 5.0);
        CHECK(united.z.min == -1.0);
        CHECK(united.z.max == 6.0);
    }
}

TEST_CASE("AABB Ray Intersection", "[bvh][aabb]") {
    AABB box(vec3(-1, -1, -1), vec3(1, 1, 1));

    SECTION("Direct central hit along -Z") {
        ray r(vec3(0, 0, 5), vec3(0, 0, -1));
        CHECK(box.intersect(r, 0.001, 100.0) == true);
    }

    SECTION("Ray pointing away misses") {
        ray r(vec3(0, 0, 5), vec3(0, 0, 1));
        CHECK(box.intersect(r, 0.001, 100.0) == false);
    }

    SECTION("Ray misses outside bounding box") {
        ray r(vec3(5, 5, 5), vec3(0, 0, -1));
        CHECK(box.intersect(r, 0.001, 100.0) == false);
    }

    SECTION("Ray starting inside bounding box") {
        ray r(vec3(0, 0, 0), vec3(1, 0, 0));
        CHECK(box.intersect(r, 0.001, 100.0) == true);
    }

    SECTION("Ray parallel to a face inside slab intersects") {
        // Parallel to Z axis, passing through x=0, y=0
        ray r(vec3(0, 0, -10), vec3(0, 0, 1));
        CHECK(box.intersect(r, 0.001, 100.0) == true);
    }

    SECTION("Ray parallel to a face outside slab misses") {
        // Parallel to Z axis, passing through x=5, y=0 (outside x interval)
        ray r(vec3(5, 0, -10), vec3(0, 0, 1));
        CHECK(box.intersect(r, 0.001, 100.0) == false);
    }
}

TEST_CASE("Shape Bounding Boxes", "[bvh][shapes]") {
    SECTION("Sphere bounding box") {
        Sphere s(vec3(1, 2, 3), 2.0f);
        AABB box = s.bounding_box();

        CHECK_THAT(box.x.min, Catch::Matchers::WithinAbs(-1.0, EPSILON));
        CHECK_THAT(box.x.max, Catch::Matchers::WithinAbs(3.0, EPSILON));
        CHECK_THAT(box.y.min, Catch::Matchers::WithinAbs(0.0, EPSILON));
        CHECK_THAT(box.y.max, Catch::Matchers::WithinAbs(4.0, EPSILON));
        CHECK_THAT(box.z.min, Catch::Matchers::WithinAbs(1.0, EPSILON));
        CHECK_THAT(box.z.max, Catch::Matchers::WithinAbs(5.0, EPSILON));
    }

    SECTION("Triangle bounding box") {
        Triangle tri(vec3(0, 0, 0), vec3(2, 0, 0), vec3(0, 3, 0));
        AABB box = tri.bounding_box();

        CHECK(box.x.min <= 0.0);
        CHECK(box.x.max >= 2.0);
        CHECK(box.y.min <= 0.0);
        CHECK(box.y.max >= 3.0);
        // Z has padding for zero-thickness planar triangle
        CHECK(box.z.size() > 0.0);
    }
}

TEST_CASE("BVH Equivalence with Linear Search", "[bvh]") {
    ShapeList linear_scene;

    // Create a deterministic set of spheres at various positions
    for (int i = -3; i <= 3; ++i) {
        for (int j = -3; j <= 3; ++j) {
            float x = i * 4.0f;
            float y = j * 4.0f;
            float z = -15.0f - (i + j);
            linear_scene.add(std::make_shared<Sphere>(vec3(x, y, z), 1.5f));
        }
    }

    // Build the BVH
    BVHNode bvh(linear_scene.objects);

    // Test a grid of rays
    int hits = 0;
    for (int px = -15; px <= 15; ++px) {
        for (int py = -15; py <= 15; ++py) {
            vec3 origin(0, 0, 0);
            vec3 dir = normalize(vec3(px * 0.5f, py * 0.5f, -10.0f));
            ray r(origin, dir);

            hit_record linear_rec;
            bool linear_hit = false;
            double closest_t = 1000.0;
            for (const auto& obj : linear_scene.objects) {
                hit_record temp;
                if (obj->intersect(r, 0.001, closest_t, temp)) {
                    linear_hit = true;
                    closest_t = temp.t;
                    linear_rec = temp;
                }
            }

            hit_record bvh_rec;
            bool bvh_hit = bvh.intersect(r, 0.001, 1000.0, bvh_rec);

            // Both must agree on hit or miss
            REQUIRE(bvh_hit == linear_hit);

            if (linear_hit) {
                hits++;
                // Both must find the exact same closest hit distance
                REQUIRE_THAT(bvh_rec.t, Catch::Matchers::WithinAbs(linear_rec.t, EPSILON));
                // Hit points must match
                REQUIRE_THAT(bvh_rec.p.x(), Catch::Matchers::WithinAbs(linear_rec.p.x(), EPSILON));
                REQUIRE_THAT(bvh_rec.p.y(), Catch::Matchers::WithinAbs(linear_rec.p.y(), EPSILON));
                REQUIRE_THAT(bvh_rec.p.z(), Catch::Matchers::WithinAbs(linear_rec.p.z(), EPSILON));
            }
        }
    }

    CHECK(hits > 0);
}

TEST_CASE("ShapeList BVH integration", "[bvh][shapelist]") {
    ShapeList scene;
    scene.add(std::make_shared<Sphere>(vec3(0, 0, -5), 1.0f));
    scene.add(std::make_shared<Sphere>(vec3(2, 0, -7), 1.0f));

    // Linear before BVH build
    CHECK(scene.bvh_root == nullptr);
    ray r(vec3(0, 0, 0), vec3(0, 0, -1));
    hit_record rec1;
    CHECK(scene.intersect(r, 0.001, 100.0, rec1) == true);
    CHECK_THAT(rec1.t, Catch::Matchers::WithinAbs(4.0, EPSILON));

    // Accelerated after BVH build
    scene.build_bvh();
    CHECK(scene.bvh_root != nullptr);
    hit_record rec2;
    CHECK(scene.intersect(r, 0.001, 100.0, rec2) == true);
    CHECK_THAT(rec2.t, Catch::Matchers::WithinAbs(4.0, EPSILON));

    // Clear BVH restores linear mode
    scene.clear_bvh();
    CHECK(scene.bvh_root == nullptr);
    hit_record rec3;
    CHECK(scene.intersect(r, 0.001, 100.0, rec3) == true);
    CHECK_THAT(rec3.t, Catch::Matchers::WithinAbs(4.0, EPSILON));
}
