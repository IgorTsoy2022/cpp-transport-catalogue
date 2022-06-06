#pragma once

#include <cmath>

const double RADIAN_PER_DEGREE = 0.017453292519;
const double THE_RADIUS_OF_THE_EARTH = 6371000.0;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    if (from == to) {
        return 0;
    }
    return std::acos(std::sin(from.lat * RADIAN_PER_DEGREE) *
                     std::sin(to.lat * RADIAN_PER_DEGREE) +
                     std::cos(from.lat * RADIAN_PER_DEGREE) *
                     std::cos(to.lat * RADIAN_PER_DEGREE) *
                     std::cos(std::abs(from.lng - to.lng) *
                     RADIAN_PER_DEGREE)) * THE_RADIUS_OF_THE_EARTH;
}