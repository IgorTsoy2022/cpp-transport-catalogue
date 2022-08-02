#include "request_handler.h"

RequestHandler::RequestHandler(const cat::TransportCatalogue& db,
    json::Reader& loaded, svg::MapRenderer& map_renderer)
    : db_(db)
    , loaded_(loaded)
    , map_renderer_(map_renderer)
{}

using namespace std::literals;

void RequestHandler::JSONout(std::ostream& out) {

    json::Array root;

    for (const auto& request : loaded_.GetStatRequests()) {
        json::Dict blocks;
        blocks["request_id"s] =
            std::move(json::Node(request.id));

        if (request.type == dom::QueryType::STOP) {
            auto stop_info = db_.GetStopInfo(request.name);
            if (stop_info.exists) {
                json::Array items;
                if (stop_info.buses.size() > 0) {

                    for (auto bus : stop_info.buses) {
                        items.push_back(
                            std::move(json::Node(
                                bus->name)));
                    }

                }
                blocks["buses"s] =
                    std::move(json::Node(std::move(
                        items)));
            }
            else {
                blocks["error_message"s] =
                    std::move(json::Node(
                        "not found"s));
            }
        }
        else if (request.type == dom::QueryType::BUS) {
            auto bus_info = db_.GetBusInfo(request.name);
            if (bus_info.route_stops > 0) {
                blocks["stop_count"s] =
                    std::move(json::Node(
                        bus_info.route_stops));
                blocks["unique_stop_count"s] =
                    std::move(json::Node(
                        bus_info.unique_stops));
                blocks["route_length"s] =
                    std::move(json::Node(bus_info.length));
                blocks["curvature"s] =
                    std::move(json::Node(bus_info.curvature));
            }
            else {
                blocks["error_message"s] =
                    std::move(json::Node("not found"s));
            }
        }
        else if (request.type == dom::QueryType::MAP) {
            std::ostringstream map_stream;
            RenderMap(map_stream);
            blocks["map"s] = std::move(json::Node(map_stream.str()));
        }
        else if (request.type == dom::QueryType::ROUTE) {
            const auto& routing_settings = 
                loaded_.GetRoutingSettings();
            if (loaded_.ReloadRoutingSettings()) {
                transport_router_.BuildGraph(db_, 
                    routing_settings);
                loaded_.ReloadRoutingSettings(false);
            }
            std::vector<dom::TripAction> actions;
            actions = transport_router_.GetRoute(request.from_stop,
                          request.to_stop, routing_settings.bus_wait_time);

            if (actions.size() > 0) {

                json::Array items;
                double total_time = 0.0;
                for (const auto& action : actions) {
                    json::Dict items_dict;
                    if (action.type == dom::ActionType::WAIT) {
                        items_dict["type"s] =
                            std::move(json::Node("Wait"s));
                        items_dict["stop_name"s] =
                            std::move(json::Node(action.name));
                        items_dict["time"s] =
                            std::move(json::Node(action.time));
                        items.push_back(json::Node(items_dict));
                        total_time += action.time;
                        continue;
                    }
                    if (action.type == dom::ActionType::IN_BUS) {
                        items_dict["type"s] =
                            std::move(json::Node("Bus"s));
                        items_dict["bus"s] =
                            std::move(json::Node(action.name));
                        items_dict["span_count"s] =
                            std::move(json::Node(action.span_count));
                        items_dict["time"s] =
                            std::move(json::Node(action.time));
                        items.push_back(json::Node(items_dict));
                        total_time += action.time;
                        continue;
                    }
                }
                blocks["items"s] =
                    std::move(json::Node(std::move(items)));
                blocks["total_time"s] =
                    std::move(json::Node(total_time));

            }
            else {
                blocks["error_message"s] =
                    std::move(json::Node("not found"s));
            }

        }

        root.push_back(std::move(json::Node(std::move(
            blocks))));
    }

    json::Document doc{ json::Builder{}
                              .Value(std::move(json::Node(root)))
                              .Build() };

    json::Print(doc, out);
}

void RequestHandler::RenderMap(std::ostream& out) {
    map_renderer_.RenderMap(db_, loaded_.GetRouteMapSettings()).Render(out);
}


void RequestHandler::TXTout(std::ostream& out, int precision) {

    for (const auto& request : loaded_.GetStatRequests()) {
        if (request.type == dom::QueryType::BUS) {
            auto bus_info = db_.GetBusInfo(request.name);
            BusInfo(bus_info, precision, out);
            continue;
        }
        if (request.type == dom::QueryType::STOP) {
            auto stop_info = db_.GetStopInfo(request.name);
            StopInfo(stop_info, out);
            continue;
        }
        if (request.type == dom::QueryType::MAP) {
            RenderMap(out);
            continue;
        }
        if (request.type == dom::QueryType::ROUTE) {
            if (loaded_.ReloadRoutingSettings()) {
                transport_router_.BuildGraph(db_,
                    loaded_.GetRoutingSettings());
                loaded_.ReloadRoutingSettings(false);
            }
            out << "ROUTE\n"sv;
            continue;
        }
        out << "Unknown request."sv << std::endl;
    }
}

void RequestHandler::BusInfo(const dom::BusInfo& bus_info,
    int precision, std::ostream& out) {
    out << std::setprecision(precision)
        << "Bus "sv << bus_info.name << ": "sv;
    if (bus_info.route_stops > 0) {
        out << bus_info.route_stops << " stops on route, "sv
            << bus_info.unique_stops << " unique stops, "sv
            << bus_info.length << " route length, "sv
            << bus_info.curvature << " curvature"sv;
    }
    else {
        out << "not found"sv;
    }
    out << std::endl;
}

void RequestHandler::StopInfo(const dom::StopInfo& stop_info,
    std::ostream& out) {
    out << "Stop "sv << stop_info.name << ": "sv;
    if (stop_info.exists) {
        if (stop_info.buses.size() > 0) {
            out << "buses"sv;
            for (const auto& bus : stop_info.buses) {
                out << " "sv << bus->name;
            }
        }
        else {
            out << "no buses"sv;
        }
    }
    else {
        out << "not found"sv;
    }
    out << std::endl;
}