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

    struct Buscomp final {
        bool operator() (const dom::Bus* left,
            const dom::Bus* right) const;
    };

    using SetBus = std::set<dom::Bus*, Buscomp>;

    struct TwoStopsHasher {
        size_t operator()(
            const std::pair<dom::Stop*, dom::Stop*> stops) const;
    };

    struct StopsDistance {
        std::string_view from_stop;
        std::string_view to_stop;
        int distance;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(const std::string_view stop_name,
            double latitude, double longitude);
        void AddStop(const dom::Stop& stop);

        void AddStopDistances(const std::string_view stop_name,
            const std::vector<std::pair<std::string, int>>&
            distances);
        void AddStopDistances(const std::vector<StopsDistance>&
            stops_distances);

        void AddBus(const std::string_view bus_name, bool is_annular,
            const std::vector<std::string>& stop_names);

        const dom::Stop* GetStop(
            const std::string_view stop_name) const;
        const dom::Bus* GetBus(
            const std::string_view bus_name) const;

        int DistanceBetweenStops(const std::string_view stop_name1,
            const std::string_view stop_name2) const;
        int DistanceBetweenStops(dom::Stop* stop1,
            dom::Stop* stop2) const;

        double RouteGeoLength(
            const std::string_view bus_name) const;
        int RouteLength(const std::string_view bus_name) const;

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
        const std::unordered_map<std::pair<dom::Stop*, dom::Stop*>,
            int, TwoStopsHasher>&
            GetDistances() const;

        dom::RouteMapSettings& GetRouteMapSettings();
        const dom::RouteMapSettings& GetRouteMapSettings() const;

        const bool RouterIsSet() const;
        const void SetRouterIsSet(bool value);

        dom::RoutingSettings& GetRoutingSettings();
        const dom::RoutingSettings& GetRoutingSettings() const;

        dom::SerializationSettings& GetSerializationSettings();
        const dom::SerializationSettings& 
        GetSerializationSettings() const;

        void Clear();

    private:
        std::deque<dom::Stop> stops_;
        std::unordered_map<std::string_view, dom::Stop*>
            stops_map_;
        std::deque<dom::Bus> buses_;
        std::unordered_map<std::string_view, dom::Bus*>
            buses_map_;

        std::unordered_map<dom::Stop*, SetBus> stop_buses_map_;
        std::unordered_map<std::pair<dom::Stop*, dom::Stop*>,
            int, TwoStopsHasher> distances_;

        dom::RouteMapSettings route_map_settings_;

        bool router_is_set_ = false;
        dom::RoutingSettings routing_settings_;

        dom::SerializationSettings serialization_settings_;

        void InsertBusesToStop(dom::Bus* bus);
    };

} // namespace cat