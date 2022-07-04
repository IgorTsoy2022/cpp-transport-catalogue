#pragma once

#include <string>
#include <vector>

namespace dom {

    struct BUS;

    struct STOP {
        std::string name;
        double latitude = 0.0;
        double longitude = 0.0;
    };

    struct BUS {
        std::string name;
        bool is_annular = false;
        std::vector<STOP*> stops;
    };

    struct BUSinfo {
        std::string name;
        int route_stops = 0;
        int unique_stops = 0;
        int length = 0;
        double curvature = 0.0;
    };

    struct STOPinfo {
        std::string name;
        bool exists = false;
        std::vector<BUS*> buses;
    };

    enum class QueryType {
        STOP,
        BUS,
        MAP,
        UNKNOWN
    };

    struct QUERY {
        QueryType type = QueryType::UNKNOWN;
        int id = 0;
        std::string name;
    };

} // namespace dom