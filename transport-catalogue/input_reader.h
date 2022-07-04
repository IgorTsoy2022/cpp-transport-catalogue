#pragma once

#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip> 
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace txt {

    using Distances =
        std::unordered_map<std::string,
        std::vector<std::pair<std::string, int>>>;
    using Routes =
        std::unordered_map<std::string,
        std::pair<bool, std::vector<std::string>>>;

    std::vector<dom::QUERY>
    QueriesToDataBase(cat::TransportCatalogue& db,
                      std::istream& in = std::cin);
    dom::QueryType GetQueryType(std::string_view value);
    void LoadStops(std::string_view name,
            std::string_view query,
            cat::TransportCatalogue& db,
            Distances& distances);
    void LoadRoutes(std::string_view name,
            std::string_view query,
            Routes& routes);

    bool IsIntNumber(std::string_view symbols);

    std::string_view Trim(std::string_view value);

    std::vector<std::string_view>
    Split(const std::string_view line, char delimiter,
              bool trimmed = false);
    void Split(const std::string_view line, char delimiter,
            std::vector<std::string>& tokens, bool trimmed = false);

} // namespace txt