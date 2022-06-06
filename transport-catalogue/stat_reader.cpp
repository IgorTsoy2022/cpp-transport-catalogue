#include "stat_reader.h"

namespace cat {
namespace print {

void Info(TransportCatalogue& db, const std::vector<QUERY>& queries,
          int precision, std::ostream& out) {
    for (const auto& query : queries) {
        if (query.type == QueryType::BUS) {
            auto bus_info = db.GetBusInfo(query.query);
            BusInfo(bus_info, precision, out);
            continue;
        }
        if (query.type == QueryType::STOP) {
            auto stop_info = db.GetStopInfo(query.query);
            StopInfo(stop_info, out);
            continue;
        }
        out << "Unknown query." << std::endl;
    }
}

void BusInfo(const BUS_Info& bus, int precision,
             std::ostream& out) {
    out << std::setprecision(precision)
        << "Bus " << bus.name << ": ";
    if (bus.route_stops > 0) {
        out << bus.route_stops << " stops on route, "
            << bus.unique_stops << " unique stops, "
            << bus.length << " route length, "
            << bus.curvature << " curvature";
    } else {
        out << "not found";
    }
    out << std::endl;
}

void StopInfo(const STOP_Info& stop, std::ostream& out) {
    out << "Stop " << stop.name << ": ";
    if (stop.exists) {
        if (stop.buses.size() > 0) {
            out << "buses";
            for (auto bus : stop.buses) {
                out << " " << bus->name;
            }
        } else {
            out << "no buses";
        }
    } else {
        out << "not found";
    }
    out << std::endl;
}

void Stops(const TransportCatalogue& tc, int precision,
           std::ostream& out) {
    for (auto& [key, coords] : tc.GetStops()) {
        out << std::setprecision(precision)
            << "Stop [" << key
            << "]: " << coords->latitude
            << ", " << coords->longitude << std::endl;
    }
}

void Buses(const TransportCatalogue& tc, std::ostream& out) {
    for (auto& [key, bus] : tc.GetBuses()) {
        out << "Bus [" << key << "]: ";
        auto count = bus->stops.size();
        for (auto& stop : bus->stops) {
            out << "[" << stop->name << "]"
                << ((--count > 0) ? " > " : "");
        }
        out << std::endl;
    }
}

void Distances(const TransportCatalogue& tc, std::ostream& out) {
    for (auto& [key, distance] : tc.GetDistances()) {
        out << "Distance between [" << key.first->name
            << "] - [" << key.second->name << "] = "
            << distance << std::endl;
    }
}

} // namespace print
} // namespace cat