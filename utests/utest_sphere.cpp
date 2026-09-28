#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <memory>

#include "shapes/shape.h"
#include "shapes/Sphere.h"
#include "shapes/Circle.h"
#include "math/ray.h"
#include "math/vec3.h"

TEST_CASE("Sphere Construction and Attributes", "[sphere][construction]") {
    SECTION("Default constructor creates unit sphere at origin") {
        Sphere s;
        REQUIRE(s.position.x() == 0.0);
        REQUIRE(s.position.y() == 0.0);
        REQUIRE(s.position.z() == 0.0);
        REQUIRE(s.radius == 1.0);
    }

    SECTION("Parameter constructor initializes center and radius") {
        point3 center(2.0, -3.0, 5.0);
        double r = 4.5;
        Sphere s(center, r);

        REQUIRE(s.center().x() == 2.0);
        REQUIRE(s.center().y() == -3.0);
        REQUIRE(s.center().z() == 5.0);
        REQUIRE(s.get_radius() == 4.5);
    }
}

TEST_CASE("Ray-Sphere Intersection - Direct Central Hit", "[sphere][intersection][hit]") {
    Sphere s(point3(0, 0, 0), 1.0);

    SECTION("Ray along -z axis through center hits sphere") {
        ray r(point3(0, 0, 5), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray along +z axis through center hits sphere") {
        ray r(point3(0, 0, -5), vec3(0, 0, 1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray along -x axis through center hits sphere") {
        ray r(point3(5, 0, 0), vec3(-1, 0, 0));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray along -y axis through center hits sphere") {
        ray r(point3(0, 5, 0), vec3(0, -1, 0));
        REQUIRE(s.intersect(r) == true);
    }
}

TEST_CASE("Ray-Sphere Intersection - Off-Center Piercing Hit", "[sphere][intersection][hit]") {
    Sphere s(point3(0, 0, 0), 2.0);

    SECTION("Ray parallel to z-axis but offset within radius hits sphere") {
        // Distance from center to ray line is sqrt(1^2 + 1^2) = sqrt(2) approx 1.414 < 2.0
        ray r(point3(1.0, 1.0, 10.0), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Diagonal ray aimed into sphere hits sphere") {
        ray r(point3(3.0, 3.0, 3.0), vec3(-1.0, -1.0, -1.0));
        REQUIRE(s.intersect(r) == true);
    }
}

TEST_CASE("Ray-Sphere Intersection - Tangent / Grazing Ray", "[sphere][intersection][hit]") {
    Sphere s(point3(0, 0, 0), 1.0);

    SECTION("Ray grazing sphere surface at x = 1 hits exactly once") {
        // Line passes through (1.0, 0.0, 0.0), exactly at sphere radius
        ray r(point3(1.0, 0.0, 5.0), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray grazing sphere surface at y = -1 hits exactly once") {
        ray r(point3(0.0, -1.0, 5.0), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == true);
    }
}

TEST_CASE("Ray-Sphere Intersection - A Miss (No Intersection)", "[sphere][intersection][miss]") {
    Sphere s(point3(0, 0, 0), 1.0);

    SECTION("Ray passing outside radius in x-axis misses") {
        // Distance to ray line is 1.5 > 1.0 (radius)
        ray r(point3(1.5, 0.0, 5.0), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == false);
    }

    SECTION("Ray passing outside radius in y-axis misses") {
        ray r(point3(0.0, 2.0, 5.0), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == false);
    }

    SECTION("Ray shooting completely away from sphere misses") {
        ray r(point3(10.0, 10.0, 10.0), vec3(1.0, 0.0, 0.0));
        REQUIRE(s.intersect(r) == false);
    }
}

TEST_CASE("Ray-Sphere Intersection - Sphere Behind Ray Origin", "[sphere][intersection][behind]") {
    Sphere s(point3(0, 0, 0), 1.0);

    SECTION("Ray pointing away from sphere does not intersect forward along ray") {
        // Ray origin at (0, 0, 5), pointing towards +z (away from origin)
        ray r(point3(0, 0, 5), vec3(0, 0, 1));
        REQUIRE(s.intersect(r) == false);
    }

    SECTION("Ray origin in front of sphere pointing further away misses") {
        ray r(point3(3, 0, 0), vec3(1, 0, 0));
        REQUIRE(s.intersect(r) == false);
    }
}

TEST_CASE("Ray-Sphere Intersection - Ray Origin Inside Sphere", "[sphere][intersection][inside]") {
    Sphere s(point3(0, 0, 0), 2.0);

    SECTION("Ray originating at sphere center hits surface on way out") {
        ray r(point3(0, 0, 0), vec3(0, 0, 1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray originating off-center inside sphere hits surface") {
        ray r(point3(0.5, -0.5, 0.5), vec3(1, 1, 0));
        REQUIRE(s.intersect(r) == true);
    }
}

TEST_CASE("Ray-Sphere Intersection - Translated Sphere Position", "[sphere][translation]") {
    // Sphere translated to center (5, 5, -5) with radius 1.5
    Sphere s(point3(5, 5, -5), 1.5);

    SECTION("Ray aimed directly at translated sphere center hits") {
        ray r(point3(5, 5, 5), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == true);
    }

    SECTION("Ray aimed at origin (0, 0, 0) misses translated sphere") {
        ray r(point3(0, 0, 5), vec3(0, 0, -1));
        REQUIRE(s.intersect(r) == false);
    }
}

TEST_CASE("Polymorphic Shape Interface and Circle Compatibility", "[shape][polymorphism]") {
    Sphere s(point3(0, 0, 0), 1.0);
    Circle c(point3(0, 0, 0), 1.0);

    ray hit_ray(point3(0, 0, 5), vec3(0, 0, -1));
    ray miss_ray(point3(5, 0, 5), vec3(0, 0, -1));

    SECTION("Virtual intersect dispatch via Shape pointer") {
        const Shape* shape_ptr = &s;
        REQUIRE(shape_ptr->intersect(hit_ray) == true);
        REQUIRE(shape_ptr->intersect(miss_ray) == false);
    }

    SECTION("Virtual intersect dispatch via Shape reference") {
        const Shape& shape_ref = s;
        REQUIRE(shape_ref.intersect(hit_ray) == true);
        REQUIRE(shape_ref.intersect(miss_ray) == false);
    }

    SECTION("Circle class functions identically as a derived Sphere") {
        REQUIRE(c.intersect(hit_ray) == true);
        REQUIRE(c.intersect(miss_ray) == false);
    }
}
