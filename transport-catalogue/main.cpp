#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    cat::TransportCatalogue db;
    json::Reader read_from_json;
    svg::MapRenderer map_renderer;
    serialization::Portal portal;

    RequestHandler request_handler{ db, read_from_json, map_renderer };

    read_from_json.LoadRequests(db, std::cin);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        std::string file_name =
            db.GetSerializationSettings().filename;
        serialization::Path file_path = std::filesystem::path(file_name);
        portal.Serialize(file_path, db);
    }
    else if (mode == "process_requests"sv) {
        std::string file_name =
            db.GetSerializationSettings().filename;
        serialization::Path file_path = std::filesystem::path(file_name);
        db.Clear();
        portal.Deserialize(file_path, db);
        request_handler.JSONout(std::cout);
    } else {
        PrintUsage();
        return 1;
    }
    
    return 0;
}
