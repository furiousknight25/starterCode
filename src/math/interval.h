#pragma once

#include <limits>

class interval {
public:
    double min;
    double max;

    interval()
        : min(+std::numeric_limits<double>::infinity()),
          max(-std::numeric_limits<double>::infinity()) {} // Default empty interval

    interval(double min, double max) : min(min), max(max) {}

    double size() const {
        return max - min;
    }

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    interval expand(double delta) const {
        auto padding = delta / 2.0;
        return interval(min - padding, max + padding);
    }

    static const interval empty;
    static const interval universe;
};

inline const interval interval::empty(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
inline const interval interval::universe(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());
