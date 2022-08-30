#pragma once

#include <string>
#include <variant>
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

    // Structures for map rendering

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r), green(g), blue(b)
        {};

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b,
            double o)
            : red(r), green(g), blue(b), opacity(o)
        {};

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string,
                               Rgb, Rgba>;

    struct RouteMapSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;

        double line_width = 0.0;

        double stop_radius = 0.0;

        int bus_label_font_size = 0;
        Point bus_label_offset{ 0.0, 0.0 };

        int stop_label_font_size = 0;
        Point stop_label_offset{ 0.0, 0.0 };

        Color underlayer_color;

        double underlayer_width = 0.0;

        std::vector<Color> color_palette;
    };

    struct RoutingSettings {
        int bus_wait_time = 6;
        double bus_velocity = 40.0;
    };

    struct SerializationSettings {
        std::string filename;
    };

} // namespace dom