#pragma once

#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "svg.h"
#include "transport_catalogue.h"

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

        const std::vector<dom::Query>& GetStatRequests() const;

    private:
        std::vector<dom::Query> stat_requests_;

        void LoadStops(const Dict& request,
            Distances& distances,
            cat::TransportCatalogue& db);
        void LoadBuses(const Dict& request,
            Buses& buses);

        void LoadRouteMapSettings(cat::TransportCatalogue& db, 
                                  const Dict& requests);
        void LoadRoutingSettings(cat::TransportCatalogue& db,
                                 const Dict& requests);
        void LoadSerializationSettings(cat::TransportCatalogue& db, 
                                       const Dict& requests);

        dom::Point GetLabelOffset(const Node& node);
        dom::Color GetColor(const Node& node);

        void LoadStatRequests(const Array& stat_requests);
    };

} // namespace json