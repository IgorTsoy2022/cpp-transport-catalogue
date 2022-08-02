#pragma once

#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace json {

    class Reader {
    public:
        using Distances =
            std::unordered_map<std::string /*from_stop*/,
            std::vector<std::pair<std::string /*to_stop*/,
                                  int /*meters*/>>>;
        using Buses =
            std::unordered_map<std::string /*bus*/,
            std::pair<bool /*is_annular*/,
            std::vector<std::string /*stop*/>>>;

        Reader() = default;

        void LoadRequests(cat::TransportCatalogue& db,
                          std::istream& in = std::cin);

        bool ReloadRoutingSettings() const;
        void ReloadRoutingSettings(bool reload);

        const cat::RoutingSettings& GetRoutingSettings() const;
        const svg::RouteMapSettings& GetRouteMapSettings() const;
        const std::vector<dom::Query>& GetStatRequests() const;

    private:
        bool reload_routing_settings_ = false;
        cat::RoutingSettings routing_settings_;
        svg::RouteMapSettings route_map_settings_;
        std::vector<dom::Query> stat_requests_;

        void LoadStops(const Dict& request,
            Distances& distances,
            cat::TransportCatalogue& db);
        void LoadBuses(const Dict& request,
            Buses& buses);

        void LoadRoutingSettings(const Dict& requests);
        void LoadRouteMapSettings(const Dict& requests);
        svg::Point GetLabelOffset(const Node& node);
        svg::Color GetColor(const Node& node);

        void LoadStatRequests(const Array& stat_requests);
    };

} // namespace json