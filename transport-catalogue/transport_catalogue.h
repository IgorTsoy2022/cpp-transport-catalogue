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

namespace cat {

    struct BUScomp final {
        bool operator() (const dom::BUS* left,
            const dom::BUS* right) const;
    };

    using SetBUS = std::set<dom::BUS*, BUScomp>;

    struct TwoStopsHasher {
        size_t operator()(const std::pair<dom::STOP*,
            dom::STOP*> stops) const;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;

        void AddStop(const std::string_view stop_name,
            double latitude, double longitude);
        void AddStop(const dom::STOP& stop);

        void AddStopDistances(const std::string_view stop_name,
            const std::vector<std::pair<std::string, int>>&
            distances);

        void AddBus(const std::string_view bus_name, bool is_annular,
            const std::vector<std::string>& stop_names);

        const dom::STOP* GetStop(
            const std::string_view stop_name) const;
        const dom::BUS* GetBus(
            const std::string_view bus_name) const;

        int DistanceBetweenStops(const std::string_view stop_name1,
            const std::string_view stop_name2) const;
        int DistanceBetweenStops(dom::STOP* stop1,
            dom::STOP* stop2) const;

        double RouteGeoLength(
            const std::string_view bus_name) const;
        int RouteLength(const std::string_view bus_name) const;

        const dom::BUSinfo GetBusInfo(
            std::string_view bus_name) const;

        const dom::STOPinfo GetStopInfo(
            std::string_view stop_name) const;

        const std::unordered_map<std::string_view, dom::STOP*>&
            GetStops() const;
        const std::unordered_map<std::string_view, dom::BUS*>&
            GetBuses() const;
        const std::unordered_map<dom::STOP*, SetBUS>&
            GetStopBuses() const;
        const std::unordered_map<std::pair<dom::STOP*, dom::STOP*>,
            int, TwoStopsHasher>&
            GetDistances() const;

        void Clear();

    private:
        std::deque<dom::STOP> stops_;
        std::unordered_map<std::string_view, dom::STOP*>
            stops_map_;
        std::deque<dom::BUS> buses_;
        std::unordered_map<std::string_view, dom::BUS*>
            buses_map_;

        std::unordered_map<dom::STOP*, SetBUS> stop_buses_map_;
        std::unordered_map<std::pair<dom::STOP*, dom::STOP*>,
            int, TwoStopsHasher> distances_;

        void InsertBusesToStop(dom::BUS* bus);
    };

} // namespace cat