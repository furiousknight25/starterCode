#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <memory>

#include "shapes/Sphere.h"
#include "shapes/Triangle.h"
#include "shapes/ShapeList.h"
#include "math/ray.h"

constexpr double EPSILON = 1e-5;

TEST_CASE("Multiple Spheres - Closest Object Occlusion", "[multishape][occlusion]") {
    // Front sphere centered at (0, 0, -2) with radius 0.5 (front surface at z = -1.5, t = 1.5)
    auto front_sphere = std::make_shared<Sphere>(point3(0, 0, -2), 0.5, color(1.0, 0.0, 0.0));
    // Back sphere centered at (0, 0, -5) with radius 1.0 (front surface at z = -4.0, t = 4.0)
    auto back_sphere = std::make_shared<Sphere>(point3(0, 0, -5), 1.0, color(0.0, 0.0, 1.0));

    ray r(point3(0, 0, 0), vec3(0, 0, -1));

    SECTION("Adding front sphere first correctly occludes back sphere") {
        ShapeList world;
        world.add(front_sphere);
        world.add(back_sphere);

        hit_record rec;
        REQUIRE(world.intersect(r, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(1.5, EPSILON));
        REQUIRE(rec.mat_color.x() == 1.0); // Red front sphere
    }

    SECTION("Adding back sphere first still selects front sphere (order independent)") {
        ShapeList world;
        world.add(back_sphere);
        world.add(front_sphere);

        hit_record rec;
        REQUIRE(world.intersect(r, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(1.5, EPSILON));
        REQUIRE(rec.mat_color.x() == 1.0); // Red front sphere
    }
}

TEST_CASE("Mixed Scene - Sphere and Triangle Occlusion", "[multishape][triangle][sphere]") {
    // Sphere at (0, 0, -2) radius 0.5 (surface at z = -1.5, t = 1.5)
    auto sphere = std::make_shared<Sphere>(point3(0, 0, -2), 0.5, color(1.0, 0.0, 0.0));
    // Triangle at z = -4 spanning x in [-2, 2], y in [-2, 2] (surface at z = -4.0, t = 4.0)
    auto tri_behind = std::make_shared<Triangle>(point3(-2, -2, -4), point3(2, -2, -4), point3(0, 2, -4), color(0.0, 1.0, 0.0));

    ray center_ray(point3(0, 0, 0), vec3(0, 0, -1));

    SECTION("Sphere occludes triangle at center ray") {
        ShapeList world;
        world.add(tri_behind);
        world.add(sphere);

        hit_record rec;
        REQUIRE(world.intersect(center_ray, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(1.5, EPSILON));
        REQUIRE(rec.mat_color.x() == 1.0); // Front sphere
    }

    SECTION("Ray passing outside front sphere hits background triangle") {
        ShapeList world;
        world.add(tri_behind);
        world.add(sphere);

        // Ray at x = 0.8 passes outside sphere radius (0.5) but hits triangle (which spans up to x = 2.0)
        ray side_ray(point3(0.8, -0.5, 0), vec3(0, 0, -1));
        hit_record rec;
        REQUIRE(world.intersect(side_ray, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(4.0, EPSILON));
        REQUIRE(rec.mat_color.y() == 1.0); // Green triangle
    }

    SECTION("Triangle in front occludes background sphere") {
        auto tri_front = std::make_shared<Triangle>(point3(-1, -1, -1), point3(1, -1, -1), point3(0, 1, -1), color(0.0, 1.0, 0.0));
        auto sphere_back = std::make_shared<Sphere>(point3(0, 0, -5), 1.0, color(1.0, 0.0, 0.0));

        ShapeList world;
        world.add(sphere_back);
        world.add(tri_front);

        hit_record rec;
        REQUIRE(world.intersect(center_ray, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(1.0, EPSILON));
        REQUIRE(rec.mat_color.y() == 1.0); // Front triangle
    }
}
