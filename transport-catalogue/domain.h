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

    enum class ActionType {
        WAIT, IN_BUS, IDLE
    };

    struct TripAction {
        ActionType type = ActionType::IDLE;
        std::string name;
        int span_count = 0;
        double time = 0.0;
    };

    enum class QueryType {
        STOP,
        BUS,
        MAP,
        ROUTE,
        UNKNOWN
    };

    struct Query {
        QueryType type = QueryType::UNKNOWN;
        int id = 0;
        std::string name;
        std::string from_stop;
        std::string to_stop;
    };

} // namespace dom