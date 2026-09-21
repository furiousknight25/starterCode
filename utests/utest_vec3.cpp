#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "math/vec3.h"

// Common floating-point tolerance threshold
constexpr double EPSILON = 1.0e-5;

TEST_CASE("Vector addition and subtraction properties", "[vec3][algebra]") { //defines the test case
    vec3 u(1.0, 2.0, 3.0);
    vec3 v(4.0, -5.0, 6.0);

    SECTION("Commutative property: u + v == v + u") { //refreshes memory every section
        vec3 sum1 = u + v;
        vec3 sum2 = v + u;

        REQUIRE_THAT(sum1.x(), Catch::Matchers::WithinAbs(sum2.x(), EPSILON));
        REQUIRE_THAT(sum1.y(), Catch::Matchers::WithinAbs(sum2.y(), EPSILON));
        REQUIRE_THAT(sum1.z(), Catch::Matchers::WithinAbs(sum2.z(), EPSILON));
    }

    SECTION("Self-subtraction yields zero vector: u - u == 0") {
        vec3 diff = u - u;

        REQUIRE_THAT(diff.x(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(diff.y(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(diff.z(), Catch::Matchers::WithinAbs(0.0, EPSILON));
    }
}

TEST_CASE("Scalar multiplication and division", "[vec3][scaling]") {
    vec3 v(2.0, -4.0, 8.0);
    double scalar = 2.5;

    SECTION("Multiplication scales all components") {
        vec3 scaled = v * scalar;

        REQUIRE_THAT(scaled.x(), Catch::Matchers::WithinAbs(5.0, EPSILON));
        REQUIRE_THAT(scaled.y(), Catch::Matchers::WithinAbs(-10.0, EPSILON));
        REQUIRE_THAT(scaled.z(), Catch::Matchers::WithinAbs(20.0, EPSILON));
    }

    SECTION("Division matches reciprocal multiplication") {
        vec3 div_result = v / 2.0;
        vec3 mul_result = v * 0.5;

        REQUIRE_THAT(div_result.x(), Catch::Matchers::WithinAbs(mul_result.x(), EPSILON));
        REQUIRE_THAT(div_result.y(), Catch::Matchers::WithinAbs(mul_result.y(), EPSILON));
        REQUIRE_THAT(div_result.z(), Catch::Matchers::WithinAbs(mul_result.z(), EPSILON));
    }
}

TEST_CASE("Dot product geometric properties", "[vec3][dot]") {
    SECTION("Perpendicular vectors yield zero") {
        vec3 unit_x(1.0, 0.0, 0.0);
        vec3 unit_y(0.0, 1.0, 0.0);

        double result = dot(unit_x, unit_y);

        REQUIRE_THAT(result, Catch::Matchers::WithinAbs(0.0, EPSILON));
    }

    SECTION("Dot product with self equals length squared") {
        vec3 v(3.0, 4.0, 5.0);

        double result = dot(v, v);

        REQUIRE_THAT(result, Catch::Matchers::WithinAbs(v.length_squared(), EPSILON));
    }
}

TEST_CASE("Cross product direction and anti-commutativity", "[vec3][cross]") {
    vec3 x_axis(1.0, 0.0, 0.0);
    vec3 y_axis(0.0, 1.0, 0.0);

    SECTION("Standard basis: X cross Y equals Z") {
        vec3 z_result = cross(x_axis, y_axis);

        REQUIRE_THAT(z_result.x(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(z_result.y(), Catch::Matchers::WithinAbs(0.0, EPSILON));
        REQUIRE_THAT(z_result.z(), Catch::Matchers::WithinAbs(1.0, EPSILON));
    }

    SECTION("Anti-commutativity: u cross v == -(v cross u)") {
        vec3 u(1.0, 2.0, 3.0);
        vec3 v(4.0, 5.0, 6.0);

        vec3 cross1 = cross(u, v);
        vec3 cross2 = -cross(v, u);

        REQUIRE_THAT(cross1.x(), Catch::Matchers::WithinAbs(cross2.x(), EPSILON));
        REQUIRE_THAT(cross1.y(), Catch::Matchers::WithinAbs(cross2.y(), EPSILON));
        REQUIRE_THAT(cross1.z(), Catch::Matchers::WithinAbs(cross2.z(), EPSILON));
    }
}

TEST_CASE("Vector length and normalization", "[vec3][normalization]") {
    SECTION("Calculates known 3-4-0 right-triangle length") {
        vec3 v(3.0, 4.0, 0.0);

        REQUIRE_THAT(v.length(), Catch::Matchers::WithinAbs(5.0, EPSILON));
    }

    SECTION("unit_vector produces unit length vector") {
        vec3 v(12.5, -3.2, 44.1);
        vec3 norm = unit_vector(v);

        REQUIRE_THAT(norm.length(), Catch::Matchers::WithinAbs(1.0, EPSILON));
    }
}
