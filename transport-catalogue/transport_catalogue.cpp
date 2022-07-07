#include "transport_catalogue.h"

namespace cat {

    using namespace std::string_literals;

    const size_t PRIME_NUMBER = 37;

    bool Buscomp::operator()
        (const dom::Bus* left, const dom::Bus* right) const {
        return left->name < right->name;
    }

    size_t TwoStopsHasher::operator()
        (const std::pair<dom::Stop*, dom::Stop*> stops) const {
        return reinterpret_cast<size_t>(stops.first) +
            reinterpret_cast<size_t>(stops.second) *
            PRIME_NUMBER;
    }

    // public:

    void TransportCatalogue::AddStop(const std::string_view stop_name,
        double latitude, double longitude) {
        dom::Stop stop;
        stop.name = std::string(stop_name);
        stop.latitude = latitude;
        stop.longitude = longitude;
        AddStop(std::move(stop));
    }

    void TransportCatalogue::AddStop(const dom::Stop& stop) {
        stops_.push_back(stop);
        stops_map_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddStopDistances(
        const std::string_view stop_name,
        const std::vector<std::pair<std::string, int>>&
        distances) {

        if (stops_map_.count(stop_name) == 0) {
            throw std::invalid_argument(
                "Invalid Stop in Distances"s);
        }

        const auto& from_stop = stops_map_.at(stop_name);
        for (const auto& value : distances) {
            if (stops_map_.count(value.first) == 0) {
                throw std::invalid_argument(
                    "Invalid Stop in Distances"s);
            }
            distances_[{from_stop, stops_map_.at(value.first)}] =
                value.second;
        }
    }

    void TransportCatalogue::AddBus(const std::string_view bus_name,
        bool is_annular,
        const std::vector<std::string>& stop_names) {

        dom::Bus bus;
        for (const auto& stop_name : stop_names) {
            if (stops_map_.count(stop_name) == 0) {
                throw std::invalid_argument(
                    "Invalid Stop in Bus"s);
            }
            bus.stops.push_back(stops_map_.at(stop_name));
        }
        bus.name = std::string(bus_name);
        bus.is_annular = is_annular;

        buses_.push_back(std::move(bus));
        buses_map_[buses_.back().name] = &buses_.back();
        InsertBusesToStop(&buses_.back());
    }

    const dom::Stop* TransportCatalogue::GetStop(
        const std::string_view stop_name) const {
        return (stops_map_.count(stop_name) > 0) ?
            stops_map_.at(stop_name) : nullptr;
    }

    const dom::Bus* TransportCatalogue::GetBus(
        const std::string_view bus_name) const {
        return (buses_map_.count(bus_name) > 0) ?
            buses_map_.at(bus_name) : nullptr;
    }

    int TransportCatalogue::DistanceBetweenStops(
        const std::string_view stop_name1,
        const std::string_view stop_name2) const {

        if (stops_map_.count(stop_name1) == 0 ||
            stops_map_.count(stop_name2) == 0) {
            throw std::invalid_argument(
                "Invalid Stop in Distances"s);
        }

        return DistanceBetweenStops(stops_map_.at(stop_name1),
            stops_map_.at(stop_name2));
    }

    int TransportCatalogue::DistanceBetweenStops(
        dom::Stop* stop1, dom::Stop* stop2) const {

        auto pair_stop = std::pair(stop1, stop2);
        if (distances_.count(pair_stop) > 0) {
            return distances_.at(pair_stop);
        }

        pair_stop = std::pair(stop2, stop1);
        if (distances_.count(pair_stop) > 0) {
            return distances_.at(pair_stop);
        }

        return 0;
    }

    double TransportCatalogue::RouteGeoLength(
        const std::string_view bus_name) const {

        if (buses_map_.count(bus_name) == 0) {
            return 0.0;
        }

        double result = 0.0;
        double prev_latitude;
        double prev_longitude;
        bool is_first_stop = true;
        for (const auto& stop : buses_map_.at(bus_name)->stops) {
            if (is_first_stop) {
                prev_latitude = stop->latitude;
                prev_longitude = stop->longitude;
                is_first_stop = false;
                continue;
            }
            result += geo::ComputeDistance(
                { prev_latitude, prev_longitude },
                { stop->latitude, stop->longitude });
            prev_latitude = stop->latitude;
            prev_longitude = stop->longitude;
        }

        return buses_map_.at(bus_name)->is_annular
            ? result : result * 2;
    }

    int TransportCatalogue::RouteLength(
        const std::string_view bus_name) const {

        if (buses_map_.count(bus_name) == 0) {
            return 0;
        }

        int result = 0;
        const auto& bus = buses_map_.at(bus_name);
        bool is_annular = bus->is_annular;
        dom::Stop* from_stop = nullptr;
        bool is_first_stop = true;
        for (const auto& stop : bus->stops) {
            if (is_first_stop) {
                from_stop = stop;
                is_first_stop = false;
                continue;
            }

            result += is_annular
                ? DistanceBetweenStops(from_stop, stop)
                : DistanceBetweenStops(from_stop, stop)
                + DistanceBetweenStops(stop, from_stop);
            from_stop = stop;
        }

        return result;
    }

    const dom::BusInfo TransportCatalogue::GetBusInfo(
        std::string_view bus_name) const {

        dom::BusInfo bus_info;
        bus_info.name = std::string(bus_name);
        if (buses_map_.count(bus_name) > 0) {
            const auto& bus = buses_map_.at(bus_name);
            const auto& stops = bus->stops;
            bus_info.route_stops = bus->is_annular
                ? static_cast<int>(stops.size())
                : static_cast<int>(stops.size()) * 2 - 1;

            std::unordered_set<dom::Stop*> tmp_stops =
            { stops.begin(), stops.end() };
            bus_info.unique_stops = static_cast<int>(tmp_stops.size());
            bus_info.length = RouteLength(bus_name);
            bus_info.curvature = bus_info.length / RouteGeoLength(bus_name);
        }

        return bus_info;
    }

    const dom::StopInfo TransportCatalogue::GetStopInfo(
        std::string_view stop_name) const {

        dom::StopInfo stop_info;
        stop_info.name = std::string(stop_name);
        stop_info.exists = (stops_map_.count(stop_name) > 0);
        if (stop_info.exists) {
            dom::Stop* stop = stops_map_.at(stop_name);
            if (stop_buses_map_.count(stop) > 0) {
                const auto& buses_at_stop = stop_buses_map_.at(stop);
                stop_info.buses = { buses_at_stop.begin(),
                                    buses_at_stop.end() };
            }
        }

        return stop_info;
    }

    const std::unordered_map<std::string_view, dom::Stop*>&
        TransportCatalogue::GetStops() const {
        return stops_map_;
    }

    const std::unordered_map<std::string_view, dom::Bus*>&
        TransportCatalogue::GetBuses() const {
        return buses_map_;
    }

    const std::unordered_map<dom::Stop*, SetBus>&
        TransportCatalogue::GetStopBuses() const {
        return stop_buses_map_;
    }

    const std::unordered_map<std::pair<dom::Stop*, dom::Stop*>,
        int, TwoStopsHasher>&
        TransportCatalogue::GetDistances() const {
        return distances_;
    }

    void TransportCatalogue::Clear() {
        for (auto& [_, bus] : stop_buses_map_) {
            bus.clear();
        }
        stop_buses_map_.clear();

        distances_.clear();

        for (auto& [_, bus] : buses_map_) {
            bus->stops.clear();
        }
        buses_map_.clear();
        buses_.clear();

        stops_map_.clear();
        stops_.clear();
    }

    // private:

    void TransportCatalogue::InsertBusesToStop(dom::Bus* bus) {
        for (const auto& stop : bus->stops) {
            stop_buses_map_[stop].insert(bus);
        }
    }

} // namespace cat