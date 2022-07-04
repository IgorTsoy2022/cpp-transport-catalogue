#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

    const double RADIAN_PER_DEGREE = 0.017453292519;  // PI / 180
    const double THE_RADIUS_OF_EARTH = 6371000.0;

    double ComputeDistance(Coordinates from, Coordinates to) {
        if (from == to) {
            return 0;
        }
        return std::acos(std::sin(from.lat * RADIAN_PER_DEGREE) *
            std::sin(to.lat * RADIAN_PER_DEGREE) +
            std::cos(from.lat * RADIAN_PER_DEGREE) *
            std::cos(to.lat * RADIAN_PER_DEGREE) *
            std::cos(std::abs(from.lng - to.lng) *
                RADIAN_PER_DEGREE)) * THE_RADIUS_OF_EARTH;
    }

}  // namespace geo