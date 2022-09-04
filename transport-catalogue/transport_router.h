#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <string_view>

namespace cat {

    class TransportRouter {
    public:

        void BuildGraph(const TransportCatalogue& db);

        std::vector<dom::TripAction>
        GetRoute(std::string_view from_stop,
                 std::string_view to_stop, double bus_wait_time);

        graph::DirectedWeightedGraph<double>& GetGraph();

        std::vector<dom::Stop*>& GetStops();
        std::unordered_map<std::string_view, size_t>& GetStopsIds();

        std::unordered_map<graph::EdgeId, std::string_view>& GetBusesNames();
        std::unordered_map<graph::EdgeId, int>& GetStopsCounts();

        std::unique_ptr<graph::Router<double>>& GetRouter();

        const bool RouterIsSet() const;
        const void SetRouterIsSet(bool value);

        dom::RoutingSettings& GetRoutingSettings();

        void Clear();

    private:
        graph::DirectedWeightedGraph<double> graph_;

        bool router_is_set_ = false;
        dom::RoutingSettings routing_settings_;

        std::vector<dom::Stop*> stops_;
        std::unordered_map<std::string_view, size_t> stops_ids_;

        std::unordered_map<graph::EdgeId, std::string_view> buses_names_;
        std::unordered_map<graph::EdgeId, int> stops_counts_;

        std::unique_ptr<graph::Router<double>> router_;

        void AddEdges(const dom::Bus* bus, const TransportCatalogue& db);
    };

} // namespace cat