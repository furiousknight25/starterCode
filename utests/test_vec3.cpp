#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "math/vec3.h"

TEST_CASE("Vector basic operations", "[vec3]") { //TEST_CASE defines a test suite, the [vec3] tag allows for running subsets of tests from the terminal
    const double eps = 1.0e-5;

    SECTION("Vector addition") { //defines a subtest, each section is a frest state from the parant Test_case
        vec3 u(1.0, 2.0, 3.0);
        vec3 v(4.0, 5.0, 6.0);
        vec3 result = u + v;

        REQUIRE_THAT(result.x(), Catch::Matchers::WithinAbs(5.0, eps));
        REQUIRE_THAT(result.y(), Catch::Matchers::WithinAbs(7.0, eps));
        REQUIRE_THAT(result.z(), Catch::Matchers::WithinAbs(9.0, eps));
    }
}
