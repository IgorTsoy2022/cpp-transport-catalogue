#include "input_reader.h"

namespace txt {

    using namespace std::string_view_literals;

    const std::string_view WHITESPACE = " \f\n\r\t\v"sv;

    std::vector<dom::Query>
    QueriesToDataBase(cat::TransportCatalogue& db,
                      std::istream& in) {
        Distances distances;
        Routes buses;
        std::vector<dom::Query> queries;
        std::string line;
        int count = 0;
        int id = 0;
        in >> count;
        ++count;
        while (count > 0) {
            std::getline(in, line);
            if (line.size() == 0) {
                continue;
            }
            --count;
            std::string_view line_sv = Trim(line);

            // Number of queries
            if (IsIntNumber(line_sv)) {
                count = std::atoi(std::string(line_sv).data());
                continue;
            }

            // Queries to add
            if (line_sv.find(':') != line_sv.npos) {
                auto tokens = Split(line_sv, ':');
                if (tokens.size() < 2) {
                    continue;
                }
                
                auto pos = tokens[0].find(' ');
                if (pos == std::string_view::npos) {
                    continue;
                }

                auto key = tokens[0].substr(0, pos);
                auto name = Trim(tokens[0].substr(pos));
                // "Stop" has top priority to add.
                if (key == "Stop"sv) {
                    LoadStops(name, tokens[1], db, distances);
                    continue;
                }
                // "Bus" must be added after all "Stop" additions.
                if (key == "Bus"sv) {
                    LoadRoutes(name, tokens[1], buses);
                    continue;
                }
                continue;
            }

            // Database queries have last priority
            dom::Query query;
            query.id = ++id;
            auto pos = line_sv.find(' ');
            if (pos == line_sv.npos) {
                query.type = GetQueryType(line_sv);
            }
            else {
                query.type = GetQueryType(line_sv.substr(0, pos));
                query.name = 
                    std::move(std::string(Trim(line_sv.substr(pos))));
            }
            queries.push_back(std::move(query));
        }

        // Add Distances to database
        for (auto& [key, stops] : distances) {
            db.AddStopDistances(key, std::move(stops));
        }

        // Add "Bus" to database
        for (auto& [key, stops] : buses) {
            db.AddBus(key, stops.first, std::move(stops.second));
        }

        return queries;
    }

    dom::QueryType GetQueryType(std::string_view value) {
        if (value == "Stop"sv) {
            return dom::QueryType::STOP;
        }
        if (value == "Bus"sv) {
            return dom::QueryType::BUS;
        }
        if (value == "Map"sv) {
            return dom::QueryType::MAP;
        }
        return dom::QueryType::UNKNOWN;
    }

    void LoadStops(std::string_view name,
        std::string_view query,
        cat::TransportCatalogue& db,
        Distances& distances) {
        auto values = Split(query, ',');
        auto value_size = values.size();
        if (value_size < 2) {
            return;
        }
        // Add to data base
        db.AddStop(name, std::atof(std::string(values[0]).data()),
            std::atof(std::string(values[1]).data()));
        if (value_size < 3) {
            return;
        }
        // Distances must be added after all "Stop" additions.
        auto& from_stop = distances[std::string(name)];
        for (int i = 2; i < value_size; ++i) {
            auto pos = values[i].find("to"sv);
            auto stop = std::string(Trim(values[i].substr(pos + 2)));
            int d = std::atoi(std::string(values[i].substr(0, pos)).data());
            from_stop.push_back({ stop, d });
        }
    }

    void LoadRoutes(std::string_view name,
        std::string_view query,
        Routes& buses) {
        bool round_trip = (query.find('-') != query.npos) ?
            true : false;
        bool annular_trip = (query.find('>') != query.npos) ?
            true : false;
        if (!round_trip && !annular_trip) {
            return;
        }
        char delimiter = annular_trip ? '>' : '-';
        std::vector<std::string> values;
        Split(query, delimiter, values, true);
        // Add to buffer
        buses[std::string(name)] = { annular_trip, std::move(values) };
    }

    bool IsIntNumber(std::string_view value) {
        if (value.empty()) {
            return false;
        }
        std::string_view::const_iterator
            start = value.cbegin();
        if (value[0] == '+' || value[0] == '-') {
            ++start;
        }

        return std::all_of(start, value.cend(),
            [](const char& c) {
                return std::isdigit(c);
            });
    }

    std::string_view Trim(std::string_view value) {
        if (value.empty()) {
            return value;
        }

        auto pos = value.find_first_not_of(WHITESPACE);
        if (pos == value.npos || pos > value.size()) {
            return std::string_view();
        }
        value.remove_prefix(pos);

        pos = value.find_last_not_of(WHITESPACE);
        if (pos < value.size() + 1) {
            value.remove_suffix(value.size() - pos - 1);
        }

        return value;
    }

    std::vector<std::string_view>
        Split(const std::string_view line, char delimiter, bool trimmed) {
        std::vector<std::string_view> tokens;
        size_t start;
        size_t end = 0;
        while ((start = line.find_first_not_of(delimiter, end))
            != line.npos) {
            end = line.find(delimiter, start);
            std::string_view token = trimmed ?
                Trim(line.substr(start, end - start)) :
                line.substr(start, end - start);
            tokens.push_back(token);
        }
        return tokens;
    }

    void Split(const std::string_view line, char delimiter,
        std::vector<std::string>& tokens, bool trimmed) {
        size_t start;
        size_t end = 0;
        while ((start = line.find_first_not_of(delimiter, end))
            != line.npos) {
            end = line.find(delimiter, start);
            std::string token = trimmed ?
                std::string(Trim(line.substr(start, end - start))) :
                std::string(line.substr(start, end - start));
            tokens.push_back(std::move(token));
        }
    }

} // namespace txt