#pragma once

/*
 * Здесь можно разместить код наполнения транспортного
 * справочника данными из JSON, а также код обработки запросов
 * к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace json {

    class TransportCatalogueData {
    public:
        using Distances =
            std::unordered_map<std::string,
            std::vector<std::pair<std::string, int>>>;
        using Routes =
            std::unordered_map<std::string,
            std::pair<bool, std::vector<std::string>>>;

        TransportCatalogueData() = default;

        void LoadRequests(cat::TransportCatalogue& db,
            std::istream& in = std::cin);

        const std::vector<dom::QUERY>& GetRequests() const;
        const svg::RouteMapSettings& GetRouteMapSettings() const;

    private:
        std::vector<dom::QUERY> stat_requests_;
        svg::RouteMapSettings route_map_settings_;

        void LoadStops(const Dict& request,
            Distances& distances,
            cat::TransportCatalogue& db);
        void LoadRoutes(const Dict& request,
            Routes& routes);

        void LoadRequests(const Array& stat_requests);
        void LoadRouteMapSettings(const Dict& requests);
    };

} // namespace json