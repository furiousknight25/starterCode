#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "shapes/Triangle.h"
#include "math/ray.h"
#include "math/vec3.h"

constexpr double EPSILON = 1e-5;

TEST_CASE("Triangle Construction and Normal Calculation", "[triangle][geometry]") {
    point3 v0(0.0, 0.0, 0.0);
    point3 v1(2.0, 0.0, 0.0);
    point3 v2(0.0, 3.0, 0.0);
    Triangle tri(v0, v1, v2);

    SECTION("Vertices match inputs") {
        REQUIRE(tri.vertex(0).x() == 0.0);
        REQUIRE(tri.vertex(1).x() == 2.0);
        REQUIRE(tri.vertex(2).y() == 3.0);
    }

    SECTION("Normal points perpendicularly along +z") {
        vec3 n = tri.normal();
        REQUIRE_THAT(n.x(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(n.y(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(n.z(), Catch::Matchers::WithinAbs(1.0, EPSILON));
    }
}

TEST_CASE("Ray-Triangle Intersection - Piercing Hit", "[triangle][intersection][hit]") {
    point3 v0(0.0, 0.0, 0.0);
    point3 v1(2.0, 0.0, 0.0);
    point3 v2(0.0, 2.0, 0.0);
    Triangle tri(v0, v1, v2);

    SECTION("Ray passing through interior of triangle hits") {
        ray r(point3(0.5, 0.5, 5.0), vec3(0.0, 0.0, -1.0));
        hit_record rec;

        REQUIRE(tri.intersect(r) == true);
        REQUIRE(tri.intersect(r, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(5.0, EPSILON));
        REQUIRE_THAT(rec.p.x(), Catch::Matchers::WithinAbs(0.5, EPSILON));
        REQUIRE_THAT(rec.p.y(), Catch::Matchers::WithinAbs(0.5, EPSILON));
        REQUIRE_THAT(rec.p.z(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE(rec.front_face == true);
        REQUIRE_THAT(rec.normal.z(), Catch::Matchers::WithinAbs(1.0, EPSILON));
    }

    SECTION("Ray hitting from backface registers hit with opposite face normal") {
        ray r(point3(0.5, 0.5, -5.0), vec3(0.0, 0.0, 1.0));
        hit_record rec;

        REQUIRE(tri.intersect(r, 0.001, 100.0, rec) == true);
        REQUIRE_THAT(rec.t, Catch::Matchers::WithinAbs(5.0, EPSILON));
        REQUIRE(rec.front_face == false);
        REQUIRE_THAT(rec.normal.z(), Catch::Matchers::WithinAbs(-1.0, EPSILON));
    }
}

TEST_CASE("Ray-Triangle Intersection - Misses Outside Boundaries", "[triangle][intersection][miss]") {
    point3 v0(0.0, 0.0, 0.0);
    point3 v1(2.0, 0.0, 0.0);
    point3 v2(0.0, 2.0, 0.0);
    Triangle tri(v0, v1, v2);

    SECTION("Ray past hypotenuse (u + v > 1) misses") {
        ray r(point3(1.5, 1.5, 5.0), vec3(0.0, 0.0, -1.0));
        hit_record rec;
        REQUIRE(tri.intersect(r) == false);
        REQUIRE(tri.intersect(r, 0.001, 100.0, rec) == false);
    }

    SECTION("Ray with negative u misses") {
        ray r(point3(-0.5, 0.5, 5.0), vec3(0.0, 0.0, -1.0));
        REQUIRE(tri.intersect(r) == false);
    }

    SECTION("Ray with negative v misses") {
        ray r(point3(0.5, -0.5, 5.0), vec3(0.0, 0.0, -1.0));
        REQUIRE(tri.intersect(r) == false);
    }

    SECTION("Ray parallel to triangle plane misses") {
        ray r(point3(0.5, 0.5, 1.0), vec3(1.0, 0.0, 0.0));
        REQUIRE(tri.intersect(r) == false);
    }

    SECTION("Ray pointing away from triangle misses (t < 0)") {
        ray r(point3(0.5, 0.5, 5.0), vec3(0.0, 0.0, 1.0));
        REQUIRE(tri.intersect(r) == false);
    }
}
