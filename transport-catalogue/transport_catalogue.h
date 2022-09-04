#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <iostream>

namespace cat {

    namespace detail {
        struct Buscomp final {
            bool operator() (const dom::Bus* left,
                const dom::Bus* right) const;
        };

        struct TwoStopsHasher {
            size_t operator()(
                const std::pair<dom::Stop*, dom::Stop*> stops) const;
        };
    }

    using SetBus = std::set<dom::Bus*, detail::Buscomp>;
    using MapStopsDistance = 
          std::unordered_map<std::pair<dom::Stop*, dom::Stop*>,
                             int, detail::TwoStopsHasher>;

    struct StopsDistance {
        std::string_view from_stop;
        std::string_view to_stop;
        int distance;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(std::string_view stop_name,
            double latitude, double longitude);
        void AddStop(const dom::Stop& stop);

        void AddStopDistances(std::string_view stop_name,
            const std::vector<std::pair<std::string, int>>&
            distances);
        void AddStopDistances(const std::vector<StopsDistance>&
            stops_distances);

        void AddBus(std::string_view bus_name, bool is_annular,
            const std::vector<std::string>& stop_names);

        const dom::Stop* GetStop(std::string_view stop_name) const;
        const dom::Bus* GetBus(std::string_view bus_name) const;

        int DistanceBetweenStops(std::string_view stop_name1,
            std::string_view stop_name2) const;
        int DistanceBetweenStops(dom::Stop* stop1,
            dom::Stop* stop2) const;

        double RouteGeoLength(std::string_view bus_name) const;
        int RouteLength(std::string_view bus_name) const;

        const dom::BusInfo GetBusInfo(
            std::string_view bus_name) const;

        const dom::StopInfo GetStopInfo(
            std::string_view stop_name) const;

        const std::unordered_map<std::string_view, dom::Stop*>&
            GetStops() const;
        const std::unordered_map<std::string_view, dom::Bus*>&
            GetBuses() const;
        const std::unordered_map<dom::Stop*, SetBus>&
            GetStopBuses() const;
        const MapStopsDistance& GetDistances() const;

        void Clear();

    private:
        std::deque<dom::Stop> stops_;
        std::unordered_map<std::string_view, dom::Stop*>
            stops_map_;
        std::deque<dom::Bus> buses_;
        std::unordered_map<std::string_view, dom::Bus*>
            buses_map_;

        std::unordered_map<dom::Stop*, SetBus> stop_buses_map_;
        MapStopsDistance distances_;

        void InsertBusesToStop(dom::Bus* bus);
    };

} // namespace cat