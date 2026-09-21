#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <type_traits>

#include "math/ray.h"
#include "math/vec3.h"

// Common floating-point tolerance threshold
constexpr double EPSILON = 1.0e-5;

TEST_CASE("Construction - A ray stores the origin and direction passed to the constructor", "[ray][construction]") {
    point3 orig(1.5, -2.0, 3.25);
    vec3 dir(0.0, 1.0, -4.5);
    ray r(orig, dir);

    SECTION("Origin coordinates match inputs") {
        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(1.5, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(-2.0, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(3.25, EPSILON));
    }

    SECTION("Direction components match inputs") {
        REQUIRE_THAT(r.direction().x(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(r.direction().y(), Catch::Matchers::WithinAbs(1.0, EPSILON));
        REQUIRE_THAT(r.direction().z(), Catch::Matchers::WithinAbs(-4.5, EPSILON));
    }
}

TEST_CASE("Evaluation of Parametric Line - verify that the at function works appropriately for specific t directions", "[ray][parametric]") {
    point3 orig(2.0, 3.0, -1.0);
    vec3 dir(1.0, -2.0, 4.0);
    ray r(orig, dir);

    SECTION("t = 0 returns the origin point") {
        point3 p = r.at(0.0);
        REQUIRE_THAT(p.x(), Catch::Matchers::WithinAbs(orig.x(), EPSILON));
        REQUIRE_THAT(p.y(), Catch::Matchers::WithinAbs(orig.y(), EPSILON));
        REQUIRE_THAT(p.z(), Catch::Matchers::WithinAbs(orig.z(), EPSILON));
    }

    SECTION("t = 1 returns origin + direction") {
        point3 p = r.at(1.0);
        point3 expected = orig + dir; // (3.0, 1.0, 3.0)
        REQUIRE_THAT(p.x(), Catch::Matchers::WithinAbs(expected.x(), EPSILON));
        REQUIRE_THAT(p.y(), Catch::Matchers::WithinAbs(expected.y(), EPSILON));
        REQUIRE_THAT(p.z(), Catch::Matchers::WithinAbs(expected.z(), EPSILON));
    }

    SECTION("Arbitrary positive values of t scale forward along the ray") {
        double t = 3.5;
        point3 p = r.at(t);
        point3 expected = orig + t * dir; // (2.0 + 3.5*1.0, 3.0 + 3.5*(-2.0), -1.0 + 3.5*4.0) = (5.5, -4.0, 13.0)
        REQUIRE_THAT(p.x(), Catch::Matchers::WithinAbs(expected.x(), EPSILON));
        REQUIRE_THAT(p.y(), Catch::Matchers::WithinAbs(expected.y(), EPSILON));
        REQUIRE_THAT(p.z(), Catch::Matchers::WithinAbs(expected.z(), EPSILON));
    }

    SECTION("Arbitrary negative values of t move backward along the ray") {
        double t = -2.5;
        point3 p = r.at(t);
        point3 expected = orig + t * dir; // (2.0 - 2.5, 3.0 + 5.0, -1.0 - 10.0) = (-0.5, 8.0, -11.0)
        REQUIRE_THAT(p.x(), Catch::Matchers::WithinAbs(expected.x(), EPSILON));
        REQUIRE_THAT(p.y(), Catch::Matchers::WithinAbs(expected.y(), EPSILON));
        REQUIRE_THAT(p.z(), Catch::Matchers::WithinAbs(expected.z(), EPSILON));
    }
}

TEST_CASE("Immutability - ensure that you have no side effects that modify the ray's origin or direction once it's been set", "[ray][immutability]") {
    point3 orig(10.0, -5.0, 2.0);
    vec3 dir(1.0, 2.0, 3.0);
    ray r(orig, dir);

    SECTION("Calling at() has no side effects on origin or direction") {
        r.at(0.0);
        r.at(1.0);
        r.at(5.0);
        r.at(-100.0);
        r.at(3.14159265);

        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(10.0, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(-5.0, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(2.0, EPSILON));

        REQUIRE_THAT(r.direction().x(), Catch::Matchers::WithinAbs(1.0, EPSILON));
        REQUIRE_THAT(r.direction().y(), Catch::Matchers::WithinAbs(2.0, EPSILON));
        REQUIRE_THAT(r.direction().z(), Catch::Matchers::WithinAbs(3.0, EPSILON));
    }

    SECTION("Mutating original vector variables does not mutate ray internals (value copy semantics)") {
        orig[0] = 999.0;
        orig[1] = 888.0;
        dir[0] = -777.0;
        dir[2] = -666.0;

        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(10.0, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(-5.0, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(2.0, EPSILON));

        REQUIRE_THAT(r.direction().x(), Catch::Matchers::WithinAbs(1.0, EPSILON));
        REQUIRE_THAT(r.direction().y(), Catch::Matchers::WithinAbs(2.0, EPSILON));
        REQUIRE_THAT(r.direction().z(), Catch::Matchers::WithinAbs(3.0, EPSILON));
    }

    SECTION("origin() and direction() return const references and at() can be called on const ray") {
        static_assert(std::is_same_v<decltype(r.origin()), const point3&>, "origin() must return an immutable const reference");
        static_assert(std::is_same_v<decltype(r.direction()), const vec3&>, "direction() must return an immutable const reference");

        const ray const_r = r;
        point3 p = const_r.at(2.0);
        REQUIRE_THAT(p.x(), Catch::Matchers::WithinAbs(12.0, EPSILON));
        REQUIRE_THAT(p.y(), Catch::Matchers::WithinAbs(-1.0, EPSILON));
        REQUIRE_THAT(p.z(), Catch::Matchers::WithinAbs(8.0, EPSILON));
    }
}

TEST_CASE("Numerical Robustness - Are points computed correct within floating-point tolerance", "[ray][robustness]") {
    point3 orig(1.0 / 3.0, 2.0 / 7.0, -5.0 / 11.0);
    vec3 dir = unit_vector(vec3(1.0, 2.0, 3.0));
    ray r(orig, dir);

    SECTION("Euclidean distance along unit direction matches t parameter within tolerance") {
        double test_t_values[] = {0.001, 0.5, 1.0, 7.89, 100.25};
        for (double t : test_t_values) {
            point3 pt = r.at(t);
            double dist = (pt - orig).length();
            REQUIRE_THAT(dist, Catch::Matchers::WithinAbs(t, EPSILON));
        }
    }

    SECTION("Small step evaluation maintains precision without underflow or drift") {
        double small_t = 1.0e-7;
        point3 pt = r.at(small_t);
        REQUIRE_THAT(pt.x(), Catch::Matchers::WithinAbs(orig.x() + small_t * dir.x(), 1.0e-12));
        REQUIRE_THAT(pt.y(), Catch::Matchers::WithinAbs(orig.y() + small_t * dir.y(), 1.0e-12));
        REQUIRE_THAT(pt.z(), Catch::Matchers::WithinAbs(orig.z() + small_t * dir.z(), 1.0e-12));
    }

    SECTION("Large step evaluation maintains precision without catastrophic cancellation") {
        double large_t = 1.0e6;
        point3 pt = r.at(large_t);
        point3 expected = orig + large_t * dir;
        REQUIRE_THAT(pt.x(), Catch::Matchers::WithinAbs(expected.x(), 1.0e-4));
        REQUIRE_THAT(pt.y(), Catch::Matchers::WithinAbs(expected.y(), 1.0e-4));
        REQUIRE_THAT(pt.z(), Catch::Matchers::WithinAbs(expected.z(), 1.0e-4));
    }
}
