#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного
 * справочника данными из JSON, а также код обработки запросов
 * к базе и формирование массива ответов в формате JSON
 */

namespace json {

    using namespace std::literals;

    void TransportCatalogueData::LoadRequests(
        cat::TransportCatalogue& db, std::istream& in) {
        Distances distances;
        Routes routes;
        Document doc = Load(in);
        for (const auto& [key, value] : doc.GetRoot().AsMap()) {
            if (key == "base_requests"sv) {
                for (const auto& base_request :
                    value.AsArray()) {
                    const Dict& request = base_request.AsMap();
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
                        LoadRoutes(request, routes);
                    }
                }
                continue;
            }
            if (key == "render_settings"sv) {
                LoadRouteMapSettings(value.AsMap());
                continue;
            }
            if (key == "stat_requests"sv) {
                LoadRequests(value.AsArray());
                continue;
            }
        }

        // Add Distances to database
        for (auto& [key, stops] : distances) {
            db.AddStopDistances(key, std::move(stops));
        }

        // Add "Bus" to database
        for (auto& [key, stops] : routes) {
            db.AddBus(key, stops.first, std::move(stops.second));
        }
    }

    const std::vector<dom::Query>&
        TransportCatalogueData::GetRequests() const {
        return stat_requests_;
    }

    const svg::RouteMapSettings& 
        TransportCatalogueData::GetRouteMapSettings() const {
        return route_map_settings_;
    }

    void TransportCatalogueData::LoadStops(const Dict& request,
        Distances& distances, cat::TransportCatalogue& db) {
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
                request.at("road_distances"s).AsMap()) {
                distances[stop.name].push_back(
                    { to_stop, distance.AsInt() });
            }
        }

        db.AddStop(std::move(stop));
    }

    void TransportCatalogueData::LoadRoutes(
        const Dict& request, Routes& routes) {
        if (request.count("name"s) == 0) {
            throw std::runtime_error("Name of bus not found"s);
        }
        std::string name = request.at("name"s).AsString();

        bool is_roundtrip = false;
        if (request.count("is_roundtrip"s) > 0) {
            is_roundtrip = request.at("is_roundtrip"s).AsBool();
        }

        routes[name].first = is_roundtrip;

        if (request.count("stops"s) > 0) {
            for (const auto& stop :
                request.at("stops"s).AsArray()) {
                routes.at(name).second.push_back(
                    stop.AsString());
            }
        }
    }

    void TransportCatalogueData::LoadRequests(
        const Array& stat_requests) {
        for (const auto& stat_request : stat_requests) {
            const Dict& request = stat_request.AsMap();
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
            else if (request_type == "Map"sv)
            {
                query.type = dom::QueryType::MAP;
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

    void TransportCatalogueData::LoadRouteMapSettings(
        const Dict& requests) {
        for (const auto& [key, value] : requests) {
            if (key == "width"sv) {
                route_map_settings_.width = value.AsDouble();
                continue;
            }
            if (key == "height"sv) {
                route_map_settings_.height = value.AsDouble();
                continue;
            }
            if (key == "padding"sv) {
                route_map_settings_.padding = value.AsDouble();
                continue;
            }
            if (key == "line_width"sv) {
                route_map_settings_.line_width =
                    value.AsDouble();
                continue;
            }
            if (key == "stop_radius"sv) {
                route_map_settings_.stop_radius =
                    value.AsDouble();
                continue;
            }
            if (key == "bus_label_font_size"sv) {
                route_map_settings_.bus_label_font_size =
                    value.AsInt();
                continue;
            }
            if (key == "bus_label_offset"sv) {
                if (value.IsArray()) {
                    const auto& arr = value.AsArray();
                    if (arr.size() > 1) {
                        route_map_settings_.bus_label_offset.x =
                            arr[0].AsDouble();
                        route_map_settings_.bus_label_offset.y =
                            arr[1].AsDouble();
                    }
                }
                continue;
            }
            if (key == "stop_label_font_size"sv) {
                route_map_settings_.stop_label_font_size
                    = value.AsInt();
                continue;
            }
            if (key == "stop_label_offset"sv) {
                if (value.IsArray()) {
                    const auto& arr = value.AsArray();
                    if (arr.size() > 1) {
                        route_map_settings_.stop_label_offset.x =
                            arr[0].AsDouble();
                        route_map_settings_.stop_label_offset.y =
                            arr[1].AsDouble();
                    }
                }
                continue;
            }
            if (key == "underlayer_color"sv) {
                if (value.IsString()) {
                    route_map_settings_.underlayer_color
                        = value.AsString();
                    continue;
                }
                if (value.IsArray()) {
                    const auto& arr = value.AsArray();
                    if (arr.size() == 3) {
                        svg::Rgb color(arr[0].AsInt(),
                            arr[1].AsInt(), arr[2].AsInt());
                        route_map_settings_.underlayer_color
                            = std::move(color);
                    }
                    else if (arr.size() == 4) {
                        svg::Rgba color(arr[0].AsInt(),
                            arr[1].AsInt(), arr[2].AsInt(),
                            arr[3].AsDouble());
                        route_map_settings_.underlayer_color
                            = std::move(color);
                    }
                    continue;
                }
                continue;
            }
            if (key == "underlayer_width"sv) {
                route_map_settings_.underlayer_width =
                    value.AsDouble();
                continue;
            }
            if (key == "color_palette"sv) {
                if (value.IsArray()) {
                    for (const auto& item : value.AsArray()) {
                        if (item.IsString()) {
                            route_map_settings_.color_palette.
                                push_back(item.AsString());
                            continue;
                        }
                        if (item.IsArray()) {
                            const auto& arr = item.AsArray();
                            if (arr.size() == 3) {
                                svg::Rgb color(arr[0].AsInt(),
                                    arr[1].AsInt(),
                                    arr[2].AsInt());
                                route_map_settings_.color_palette.
                                    push_back(std::move(color));
                            }
                            else if (arr.size() == 4) {
                                svg::Rgba color(arr[0].AsInt(),
                                    arr[1].AsInt(),
                                    arr[2].AsInt(),
                                    arr[3].AsDouble());
                                route_map_settings_.color_palette.
                                    push_back(std::move(color));
                            }
                            continue;
                        }
                    }
                }
                continue;
            }
        }
    }

} // namespace json