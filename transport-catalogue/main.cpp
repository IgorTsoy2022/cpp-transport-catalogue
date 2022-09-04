#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <iostream>

using namespace std::literals;

/*
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    json::Reader read_from_json;
    cat::TransportCatalogue db;
    svg::MapRenderer map_renderer;
    cat::TransportRouter transport_router;
    serialization::Portal portal;

    RequestHandler request_handler{ read_from_json,
                                    db,
                                    map_renderer,
                                    transport_router };

    read_from_json.LoadRequests(db, map_renderer, transport_router, portal,
                                std::cin);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        std::string file_name = portal.GetSerializationSettings().filename;
        srlz::Path file_path = std::filesystem::path(file_name);
        portal.Serialize(file_path, db, map_renderer, transport_router);
    }
    else if (mode == "process_requests"sv) {
        std::string file_name = portal.GetSerializationSettings().filename;
        srlz::Path file_path = std::filesystem::path(file_name);
        db.Clear();
        portal.Deserialize(file_path, db, map_renderer, transport_router);
        request_handler.JSONout(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}
*/

int main() {

    json::Reader read_from_json;
    cat::TransportCatalogue db;
    svg::MapRenderer map_renderer;
    cat::TransportRouter transport_router;
    serialization::Portal portal;

    RequestHandler request_handler{ read_from_json,
                                    db,
                                    map_renderer,
                                    transport_router };

    std::string base_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_in_make.txt"s;
    std::ifstream jsonfile(base_file_name);
    if (!jsonfile.is_open()) {
        std::cout << "File " << base_file_name << " not found!\n";
        return 1;
    }

    read_from_json.LoadRequests(db, map_renderer, transport_router, portal, jsonfile);
    jsonfile.close();

//    std::string db_file_name = portal.GetSerializationSettings().filename;
    std::string db_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\transport_catalogue.db"s;
    serialization::Path file_path = std::filesystem::path(db_file_name);
    portal.Serialize(file_path, db, map_renderer, transport_router);

    std::string request_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_in_req.txt"s;
    jsonfile.open(request_file_name);
    if (!jsonfile.is_open()) {
        std::cout << "File " << request_file_name << " not found!\n";
        return 1;
    }

    read_from_json.LoadRequests(db, map_renderer, transport_router, portal, jsonfile);
    jsonfile.close();

    db.Clear();
    portal.Deserialize(file_path, db, map_renderer, transport_router);

    setlocale(LC_ALL, "Russian");

    std::cout << "JSON out:\n"s;

    std::string out_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_out-des.txt"s;
    std::ofstream outfile(out_file_name);
    if (outfile.is_open()) {
        request_handler.JSONout(outfile);
        outfile.close();
    }

    std::string svg_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_out-des.svg"s;
    outfile.open(svg_file_name);
    if (outfile.is_open()) {
        request_handler.RenderMap(outfile);
        outfile.close();
    }

    std::cout << "TXT out:\n"s;
    request_handler.TXTout(std::cout, 6);

    return 0;
}