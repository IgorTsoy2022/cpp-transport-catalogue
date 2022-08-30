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
                db.SetRouterIsSet(false);
                continue;
            }
            if (key == "render_settings"sv) {
                LoadRouteMapSettings(db, value.AsDict());
                continue;
            }
            if (key == "routing_settings"sv) {
                LoadRoutingSettings(db, value.AsDict());
                db.SetRouterIsSet(false);
                continue;
            }
            if (key == "serialization_settings"sv) {
                LoadSerializationSettings(db, value.AsDict());
                continue;
            }
            if (key == "stat_requests"sv) {
                LoadStatRequests(value.AsArray());
                continue;
            }
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

    void Reader::LoadRouteMapSettings(cat::TransportCatalogue& db, 
                                      const Dict& requests) {

        auto& route_map_settings = db.GetRouteMapSettings();

        for (const auto& [key, node] : requests) {
            if (key == "width"sv) {
                route_map_settings.width = node.AsDouble();
                continue;
            }
            if (key == "height"sv) {
                route_map_settings.height = node.AsDouble();
                continue;
            }
            if (key == "padding"sv) {
                route_map_settings.padding = node.AsDouble();
                continue;
            }
            if (key == "line_width"sv) {
                route_map_settings.line_width = node.AsDouble();
                continue;
            }
            if (key == "stop_radius"sv) {
                route_map_settings.stop_radius =
                    node.AsDouble();
                continue;
            }
            if (key == "bus_label_font_size"sv) {
                route_map_settings.bus_label_font_size =
                    node.AsInt();
                continue;
            }
            if (key == "bus_label_offset"sv) {
                route_map_settings.bus_label_offset =
                    std::move(GetLabelOffset(node));
                continue;
            }
            if (key == "stop_label_font_size"sv) {
                route_map_settings.stop_label_font_size
                    = node.AsInt();
                continue;
            }
            if (key == "stop_label_offset"sv) {
                route_map_settings.stop_label_offset =
                    std::move(GetLabelOffset(node));
                continue;
            }
            if (key == "underlayer_color"sv) {
                route_map_settings.underlayer_color =
                    std::move(GetColor(node));
                continue;
            }
            if (key == "underlayer_width"sv) {
                route_map_settings.underlayer_width =
                    node.AsDouble();
                continue;
            }
            if (key == "color_palette"sv) {
                if (node.IsArray()) {
                    auto size = node.AsArray().size();
                    route_map_settings.color_palette.resize(size);
                    size_t i = 0;
                    for (const auto& item : node.AsArray()) {
                        route_map_settings.color_palette[i++] = 
                            GetColor(item);
                    }
                }
                continue;
            }
        }
    }

    void Reader::LoadRoutingSettings(cat::TransportCatalogue& db,
        const Dict& requests) {

        auto& routing_settings = db.GetRoutingSettings();

        for (const auto& [key, node] : requests) {
            if (key == "bus_wait_time"sv) {
                routing_settings.bus_wait_time = node.AsInt();
                continue;
            }
            if (key == "bus_velocity"sv) {
                routing_settings.bus_velocity = node.AsDouble();
                continue;
            }
        }
    }

    void Reader::LoadSerializationSettings(cat::TransportCatalogue& db, 
                                           const Dict& requests) {

        auto& serialization_settings = db.GetSerializationSettings();

        for (const auto& [key, node] : requests) {
            if (key == "file"sv) {
                serialization_settings.filename = node.AsString();
                continue;
            }
        }
    }

    dom::Point Reader::GetLabelOffset(const Node& node) {
        dom::Point point;
        if (node.IsArray()) {
            const Array arr = node.AsArray();
            if (arr.size() > 1) {
                point.x = arr[0].AsDouble();
                point.y = arr[1].AsDouble();
            }
        }
        return point;
    }

    dom::Color Reader::GetColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        }

        if (node.IsArray()) {
            const Array arr = node.AsArray();
            if (arr.size() == 3) {
                dom::Rgb color(arr[0].AsInt(),
                    arr[1].AsInt(),
                    arr[2].AsInt());
                return color;
            }
            if (arr.size() == 4) {
                dom::Rgba color(arr[0].AsInt(),
                    arr[1].AsInt(),
                    arr[2].AsInt(),
                    arr[3].AsDouble());
                return color;
            }
        }
        return dom::Color{};
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