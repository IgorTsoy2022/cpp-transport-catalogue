#include "request_handler.h"

RequestHandler::RequestHandler(const cat::TransportCatalogue& db,
    const json::TransportCatalogueData& loaded)
    : db_(db)
    , loaded_(loaded)
{}

using namespace std::literals;

void RequestHandler::JSONout(std::ostream& out) {

    json::Array root;

    for (const auto& request : loaded_.GetRequests()) {
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
            svg::RenderMap(db_, loaded_.GetRouteMapSettings())
                .Render(map_stream);
            blocks["map"s] = std::move(json::Node(map_stream.str()));
        }

        root.push_back(std::move(json::Node(std::move(
            blocks))));
    }

    json::Document doc{ std::move(json::Node(root)) };
    json::Print(doc, out);
}

namespace cat {

    void TXTout(const cat::TransportCatalogue& db,
        const std::vector<dom::QUERY>& requests,
        int precision, std::ostream& out) {
        for (const auto& request : requests) {
            if (request.type == dom::QueryType::BUS) {
                auto bus_info = db.GetBusInfo(request.name);
                BusInfo(bus_info, precision, out);
                continue;
            }
            if (request.type == dom::QueryType::STOP) {
                auto stop_info = db.GetStopInfo(request.name);
                StopInfo(stop_info, out);
                continue;
            }
            out << "Unknown request."sv << std::endl;
        }
    }

    void BusInfo(const dom::BUSinfo& bus_info, int precision,
        std::ostream& out) {
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

    void StopInfo(const dom::STOPinfo& stop_info, std::ostream& out) {
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

    void Stops(const TransportCatalogue& db, int precision,
        std::ostream& out) {
        for (auto& [key, coords] : db.GetStops()) {
            out << std::setprecision(precision)
                << "Stop ["sv << key
                << "]: "sv << coords->latitude
                << ", "sv << coords->longitude << std::endl;
        }
    }

    void Buses(const TransportCatalogue& db,
        std::ostream& out) {
        for (auto& [key, bus] : db.GetBuses()) {
            out << "Bus ["sv << key << "]: "sv;
            auto count = bus->stops.size();
            for (auto& stop : bus->stops) {
                out << "["sv << stop->name << "]"sv
                    << ((--count > 0) ? " > "sv : ""sv);
            }
            out << std::endl;
        }
    }

    void Distances(const TransportCatalogue& db,
        std::ostream& out) {
        for (auto& [key, distance] : db.GetDistances()) {
            out << "Distance between ["sv << key.first->name
                << "] - ["sv << key.second->name << "] = "sv
                << distance << std::endl;
        }
    }

} // namespace cat