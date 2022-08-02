#include "json_reader.h"

namespace json {

    using namespace std::literals;

    // Reader: public

    void Reader::LoadRequests(cat::TransportCatalogue& db,
                              std::istream& in) {
        Distances distances;
        Buses buses;
        Document doc = Load(in);
        for (const auto& [key, value] : doc.GetRoot().AsDict()) {
            if (key == "base_requests"sv) {
                for (const auto& base_request :
                    value.AsArray()) {
                    const Dict& request = base_request.AsDict();
                    if (request.count("type"s) == 0) {
                        throw std::runtime_error(
                            "Type of data not found"s);
                    }

                    const std::string_view request_type =
                        request.at("type"s).AsString();
                    if (request_type == "Stop"sv) {
                        // "Stop" has top priority to add.
                        LoadStops(request, distances, db);
                        continue;
                    }

                    if (request_type == "Bus"sv) {
                        // "Bus" must be added after all
                        // "Stop" additions.
                        LoadBuses(request, buses);
                    }
                }
                reload_routing_settings_ = true;
                continue;
            }
            if (key == "routing_settings"sv) {
                LoadRoutingSettings(value.AsDict());
                reload_routing_settings_ = true;
                continue;
            }
            if (key == "render_settings"sv) {
                LoadRouteMapSettings(value.AsDict());
                continue;
            }
            if (key == "stat_requests"sv) {
                LoadStatRequests(value.AsArray());
                continue;
            }
        }

        if (!reload_routing_settings_) {
            return;
        }

        // Add Distances to database
        for (auto& [key, stops] : distances) {
            db.AddStopDistances(key, std::move(stops));
        }

        // Add "Bus" to database
        for (auto& [key, stops] : buses) {
            db.AddBus(key, stops.first, std::move(stops.second));
        }
    }

    bool Reader::ReloadRoutingSettings() const {
        return reload_routing_settings_;
    }

    void Reader::ReloadRoutingSettings(bool reload) {
        reload_routing_settings_ = reload;
    }

    const cat::RoutingSettings& 
        Reader::GetRoutingSettings() const {
        return routing_settings_;
    }

    const svg::RouteMapSettings&
        Reader::GetRouteMapSettings() const {
        return route_map_settings_;
    }

    const std::vector<dom::Query>&
        Reader::GetStatRequests() const {
        return stat_requests_;
    }

    // Reader: private

    void Reader::LoadStops(const Dict& request,
                           Distances& distances,
                           cat::TransportCatalogue& db) {
        if (request.count("name"s) == 0) {
            throw std::runtime_error("Name of stop not found"s);
        }
        dom::Stop stop;
        stop.name = request.at("name"s).AsString();
        if (request.count("latitude"s) > 0 &&
            request.count("longitude"s) > 0) {
            stop.latitude = request.at("latitude"s).AsDouble();
            stop.longitude = request.at("longitude"s).AsDouble();
        }

        if (request.count("road_distances"s) > 0) {
            for (const auto& [to_stop, distance] :
                request.at("road_distances"s).AsDict()) {
                distances[stop.name].push_back(
                    { to_stop, distance.AsInt() });
            }
        }

        db.AddStop(std::move(stop));
    }

    void Reader::LoadBuses(const Dict& request, Buses& buses) {
        if (request.count("name"s) == 0) {
            throw std::runtime_error("Name of bus not found"s);
        }
        std::string name = request.at("name"s).AsString();

        bool is_roundtrip = false;
        if (request.count("is_roundtrip"s) > 0) {
            is_roundtrip = request.at("is_roundtrip"s).AsBool();
        }

        buses[name].first = is_roundtrip;

        if (request.count("stops"s) > 0) {
            for (const auto& stop :
                request.at("stops"s).AsArray()) {
                buses.at(name).second.push_back(
                    stop.AsString());
            }
        }
    }

    void Reader::LoadRoutingSettings(const Dict& requests) {
        for (const auto& [key, node] : requests) {
            if (key == "bus_wait_time"sv) {
                routing_settings_.bus_wait_time = node.AsInt();
                continue;
            }
            if (key == "bus_velocity"sv) {
                routing_settings_.bus_velocity = node.AsDouble();
                continue;
            }
        }
    }

    void Reader::LoadRouteMapSettings(const Dict& requests) {
        for (const auto& [key, node] : requests) {
            if (key == "width"sv) {
                route_map_settings_.width = node.AsDouble();
                continue;
            }
            if (key == "height"sv) {
                route_map_settings_.height = node.AsDouble();
                continue;
            }
            if (key == "padding"sv) {
                route_map_settings_.padding = node.AsDouble();
                continue;
            }
            if (key == "line_width"sv) {
                route_map_settings_.line_width = node.AsDouble();
                continue;
            }
            if (key == "stop_radius"sv) {
                route_map_settings_.stop_radius =
                    node.AsDouble();
                continue;
            }
            if (key == "bus_label_font_size"sv) {
                route_map_settings_.bus_label_font_size =
                    node.AsInt();
                continue;
            }
            if (key == "bus_label_offset"sv) {
                route_map_settings_.bus_label_offset =
                    std::move(GetLabelOffset(node));
                continue;
            }
            if (key == "stop_label_font_size"sv) {
                route_map_settings_.stop_label_font_size
                    = node.AsInt();
                continue;
            }
            if (key == "stop_label_offset"sv) {
                route_map_settings_.stop_label_offset =
                    std::move(GetLabelOffset(node));
                continue;
            }
            if (key == "underlayer_color"sv) {
                route_map_settings_.underlayer_color =
                    std::move(GetColor(node));
                continue;
            }
            if (key == "underlayer_width"sv) {
                route_map_settings_.underlayer_width =
                    node.AsDouble();
                continue;
            }
            if (key == "color_palette"sv) {
                if (node.IsArray()) {
                    for (const auto& item : node.AsArray()) {
                        route_map_settings_.color_palette.
                            push_back(GetColor(item));
                    }
                }
                continue;
            }
        }
    }

    svg::Point Reader::GetLabelOffset(const Node& node) {
        svg::Point point;
        if (node.IsArray()) {
            const Array arr = node.AsArray();
            if (arr.size() > 1) {
                point.x = arr[0].AsDouble();
                point.y = arr[1].AsDouble();
            }
        }
        return point;
    }

    svg::Color Reader::GetColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        }

        if (node.IsArray()) {
            const Array arr = node.AsArray();
            if (arr.size() == 3) {
                svg::Rgb color(arr[0].AsInt(),
                               arr[1].AsInt(),
                               arr[2].AsInt());
                return color;
            }
            if (arr.size() == 4) {
                svg::Rgba color(arr[0].AsInt(),
                                arr[1].AsInt(),
                                arr[2].AsInt(),
                                arr[3].AsDouble());
                return color;
            }
        }
        return svg::Color{};
    }

    void Reader::LoadStatRequests(const Array& stat_requests) {
        for (const auto& stat_request : stat_requests) {
            const Dict& request = stat_request.AsDict();
            if (request.count("type"s) == 0) {
                throw std::runtime_error(
                    "Type of request not found"s);
            }

            dom::Query query;

            const std::string_view
                request_type = request.at("type"s).AsString();

            if (request_type == "Stop"sv) {
                query.type = dom::QueryType::STOP;
            }
            else if (request_type == "Bus"sv) {
                query.type = dom::QueryType::BUS;
            }
            else if (request_type == "Map"sv) {
                query.type = dom::QueryType::MAP;
            }
            else if (request_type == "Route"sv) {
                query.type = dom::QueryType::ROUTE;

                if (request.count("from"s) > 0) {
                    query.from_stop =
                        request.at("from"s).AsString();
                }
                
                if (request.count("to"s) > 0) {
                    query.to_stop = 
                        request.at("to"s).AsString();
                }

            }

            if (request.count("id"s) > 0) {
                query.id = request.at("id"s).AsInt();
            }

            if (request.count("name"s) > 0) {
                query.name = request.at("name"s).AsString();
            }

            stat_requests_.push_back(std::move(query));
        }
    }

} // namespace json