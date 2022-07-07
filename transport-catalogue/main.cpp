#include "input_reader.h"
#include "json_reader.h"
#include "request_handler.h"

#include <fstream>

int main() {

    using namespace std::literals;

    cat::TransportCatalogue db;
    json::TransportCatalogueData tcd;

    RequestHandler request_handler{ db, tcd };

    tcd.LoadRequests(db, std::cin);

    request_handler.JSONout(std::cout);

/*
    std::ifstream txtfile("C:\\CPP\\Yandex.Cpp\\Sprint_10\\test1.txt"s);
    if (txtfile.is_open()) {
        cat::TransportCatalogue db;
        const auto& requests = txt::QueriesToDataBase(db, txtfile);
        cat::TXTout(db, requests, 6, std::cout);
        txtfile.close();
    }
*/

/*
    std::ifstream jsonfile1("C:\\CPP\\Yandex.Cpp\\Sprint_10\\testJSONin15-3.txt"s);
    if (jsonfile1.is_open()) {
        cat::TransportCatalogue db;
        json::TransportCatalogueData tcd;

        RequestHandler request_handler{ db, tcd };

        tcd.LoadRequests(db, jsonfile1);
        jsonfile1.close();

        setlocale(LC_ALL, "Russian");

        std::cout << "TXT out:\n"s;
        cat::TXTout(db, tcd.GetRequests(), 6, std::cout);

        std::cout << "JSON out:\n"s;
        request_handler.JSONout(std::cout);

        std::ofstream outfile1("C:\\CPP\\Yandex.Cpp\\Sprint_10\\testJSONout15-3.svg"s);
        if (outfile1.is_open()) {
            svg::RenderMap(db, tcd.GetRouteMapSettings()).Render(outfile1);
            outfile1.close();
        }
    }
*/
    return 0;
}