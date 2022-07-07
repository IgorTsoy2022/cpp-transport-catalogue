#pragma once

#include <string>
#include <vector>

namespace dom {

    struct Bus;

    struct Stop {
        std::string name;
        double latitude = 0.0;
        double longitude = 0.0;
    };

    struct Bus {
        std::string name;
        bool is_annular = false;
        std::vector<Stop*> stops;
    };

    struct BusInfo {
        std::string name;
        int route_stops = 0;
        int unique_stops = 0;
        int length = 0;
        double curvature = 0.0;
    };

    struct StopInfo {
        std::string name;
        bool exists = false;
        std::vector<Bus*> buses;
    };

    enum class QueryType {
        STOP,
        BUS,
        MAP,
        UNKNOWN
    };

    struct Query {
        QueryType type = QueryType::UNKNOWN;
        int id = 0;
        std::string name;
    };

} // namespace dom