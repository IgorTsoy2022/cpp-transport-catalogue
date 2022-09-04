#include "request_handler.h"

RequestHandler::RequestHandler(json::Reader& reader,
                               cat::TransportCatalogue& db,
                               svg::MapRenderer& map_renderer,
                               cat::TransportRouter& transport_router)
    : reader_(reader)
    , db_(db)
    , map_renderer_(map_renderer)
    , transport_router_(transport_router)
{}

/*
json::Reader& RequestHandler::GetReader() {
    return reader_;
}

cat::TransportCatalogue& RequestHandler::GetTransportCatalogue() {
    return db_;
}

svg::MapRenderer& RequestHandler::GetMapRenderer() {
    return map_renderer_;
}

cat::TransportRouter& RequestHandler::GetTransportRouter() {
    return transport_router_;
}
*/

const void RequestHandler::BuildRouter() const {
    transport_router_.BuildGraph(db_);
    transport_router_.SetRouterIsSet(true);
}

using namespace std::literals;

const void RequestHandler::JSONout(std::ostream& out) const {

    json::Array root;

    for (const auto& request : reader_.GetStatRequests()) {
        json::Dict blocks;
        blocks["request_id"s] =
            std::move(json::Node(request.id));

        if (request.type == dom::QueryType::STOP) {
            StopInfo(request, blocks);
        }
        else if (request.type == dom::QueryType::BUS) {
            BusInfo(request, blocks);
        }
        else if (request.type == dom::QueryType::MAP) {
            std::ostringstream map_stream;
            RenderMap(map_stream);
            blocks["map"s] = std::move(json::Node(map_stream.str()));
        }
        else if (request.type == dom::QueryType::ROUTE) {
            RouterInfo(request, blocks);
        }

        root.push_back(std::move(json::Node(std::move(
            blocks))));
    }

    json::Document doc{ json::Builder{}
                              .Value(std::move(json::Node(root)))
                              .Build() };

    json::Print(doc, out);
}

const void RequestHandler::StopInfo(const dom::Query& request,
    json::Dict& blocks) const {

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

const void RequestHandler::BusInfo(const dom::Query& request,
    json::Dict& blocks) const {

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

const void RequestHandler::RouterInfo(const dom::Query& request,
    json::Dict& blocks) const {

    const auto& routing_settings =
        transport_router_.GetRoutingSettings();
    if (!transport_router_.RouterIsSet()) {
        transport_router_.BuildGraph(db_);
        transport_router_.SetRouterIsSet(true);
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

const void RequestHandler::RenderMap(std::ostream& out) const {
    map_renderer_.RenderMap(db_).Render(out);
}

const void RequestHandler::TXTout(std::ostream& out, int precision) const {

    for (const auto& request : reader_.GetStatRequests()) {
        out << "request_id : "sv << request.id << std::endl;

        if (request.type == dom::QueryType::STOP) {
            StopInfo(request, out);
            continue;
        }
        if (request.type == dom::QueryType::BUS) {
            BusInfo(request, precision, out);
            continue;
        }
        if (request.type == dom::QueryType::MAP) {
            RenderMap(out);
            continue;
        }
        if (request.type == dom::QueryType::ROUTE) {
            RouterInfo(request, out);
            continue;
        }
        out << "Unknown request."sv << std::endl;
    }
}

const void RequestHandler::StopInfo(const dom::Query& request,
    std::ostream& out) const {

    auto stop_info = db_.GetStopInfo(request.name);
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

const void RequestHandler::BusInfo(const dom::Query& request,
    int precision, std::ostream& out) const {

    auto bus_info = db_.GetBusInfo(request.name);
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

const void RequestHandler::RouterInfo(const dom::Query& request,
    std::ostream& out) const {

    const auto& routing_settings =
        transport_router_.GetRoutingSettings();

    if (!transport_router_.RouterIsSet()) {
        transport_router_.BuildGraph(db_);
        transport_router_.SetRouterIsSet(true);
    }

    std::vector<dom::TripAction> actions;
    actions = transport_router_.GetRoute(request.from_stop,
        request.to_stop, routing_settings.bus_wait_time);

    if (actions.size() > 0) {
        double total_time = 0.0;
        out << "Items : \n"sv;
        for (const auto& action : actions) {
            if (action.type == dom::ActionType::WAIT) {
                out << "  type       : Wait,\n"sv;
                out << "  stop_name  : "sv << action.name << ",\n"sv;
                out << "  time       : "sv << action.time << "\n"sv;
                total_time += action.time;
                continue;
            }
            if (action.type == dom::ActionType::IN_BUS) {
                out << "  type       : Bus,\n"sv;
                out << "  bus        : "sv << action.name << ",\n"sv;
                out << "  span_count : "sv << action.span_count << ",\n"sv;
                out << "  time       : "sv << action.time << "\n"sv;
                total_time += action.time;
                continue;
            }
        }
        out << "total_time : "sv << total_time << "\n"sv;
    }
    else {
        out << "error_message : not found\n"sv;
    }
}