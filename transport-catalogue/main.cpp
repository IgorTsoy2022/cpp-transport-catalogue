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

    cat::TransportCatalogue db;
    json::Reader read_from_json;
    svg::MapRenderer map_renderer;
    srlz::Portal portal;

    RequestHandler request_handler{ db, read_from_json, map_renderer };

    read_from_json.LoadRequests(db, std::cin);

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        std::string file_name = db.GetSerializationSettings().filename;
        srlz::Path file_path = std::filesystem::path(file_name);
        portal.Serialize(file_path, db);
    }
    else if (mode == "process_requests"sv) {
        std::string file_name = db.GetSerializationSettings().filename;
        srlz::Path file_path = std::filesystem::path(file_name);
        db.Clear();
        portal.Deserialize(file_path, db);
        request_handler.JSONout(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}
*/

int main() {

    dom::Color color_blank;
    dom::Color color_none = std::monostate{};
    dom::Color color_str = "Red"s;
    dom::Rgb rgb = { 2, 3, 8 };
    dom::Color color_rgb = rgb;

    std::cout << "blank index="s << color_blank.index() << "\n"s;
    std::cout << "color_none index="s << color_none.index() << "\n"s;
    std::cout << "color_str index="s << color_str.index() << "\n"s;
    std::cout << "color_rgb index="s << color_rgb.index() << "\n"s;

    dom::Rgb out = std::get<2>(color_rgb);
    int red = out.red;
    std::cout << "r="s << red << std::endl;

    std::string str = "s"s;

    std::string strout = std::get<1>(color_str);
    std::cout << "strout="s << strout << std::endl;

    const auto& color_type = serialization::GetVariantType<dom::Color>(color_str);

    std::cout << "type="s << color_type.name() << std::endl;
    std::cout << "type="s << typeid(str).name() << std::endl;
    std::cout << " ? " << (color_type.name() == typeid(str).name()) << std::endl;

//    auto info = serialization::GetVariantType(color_str);

/*
    {
        cat::TransportCatalogue db;
        json::Reader read_from_json;
        svg::MapRenderer map_renderer;

        RequestHandler request_handler{ db, read_from_json, map_renderer };

        std::string base_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC_test2-in.txt"s;
        std::ifstream jsonfile(base_file_name);
        if (!jsonfile.is_open()) {
            std::cout << "File " << base_file_name << " not found!\n";
            return 1;
        }

        read_from_json.LoadRequests(db, jsonfile);
        jsonfile.close();

        std::string out_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC_test2-out.txt"s;
        std::ofstream outfile(out_file_name);
        if (outfile.is_open()) {
            request_handler.JSONout(outfile);
            outfile.close();
        }

        std::string svg_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC_test2-out.svg"s;
        outfile.open(svg_file_name);
        if (outfile.is_open()) {
            request_handler.RenderMap(outfile);
            outfile.close();
        }

    }
*/
    cat::TransportCatalogue db;
    json::Reader read_from_json;
    svg::MapRenderer map_renderer;
    serialization::Portal portal;

    RequestHandler request_handler{ db, read_from_json, map_renderer };

    std::string base_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_in_make.txt"s;
    std::ifstream jsonfile(base_file_name);
    if (!jsonfile.is_open()) {
        std::cout << "File " << base_file_name << " not found!\n";
        return 1;
    }

    read_from_json.LoadRequests(db, jsonfile);
    jsonfile.close();

//    std::string db_file_name = db.GetSerializationSettings().filename;
    std::string db_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\transport_catalogue.db"s;
    serialization::Path file_path = std::filesystem::path(db_file_name);
    portal.Serialize(file_path, db);

    std::string request_file_name = "C:\\CPP\\Yandex.Cpp\\Sprint_14\\TC3_2_in_req.txt"s;
    jsonfile.open(request_file_name);
    if (!jsonfile.is_open()) {
        std::cout << "File " << request_file_name << " not found!\n";
        return 1;
    }

    read_from_json.LoadRequests(db, jsonfile);
    jsonfile.close();

    db.Clear();
    portal.Deserialize(file_path, db);

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

    return 0;
}