#include "request_handler.h"

#include <fstream>
#include <iostream>

using namespace std::string_literals;

int main() {
    /*
        json::Print(
            json::Document{
                json::Builder{}
                .StartDict()
                    .Key("key1"s).Value(123)
                    .Key("key2"s).Value("value2"s)
                    .Key("key3"s).StartArray()
                        .Value(456)
                        .StartDict().EndDict()
                        .StartDict()
                            .Key(""s)
                            .Value(nullptr)
                        .EndDict()
                        .Value(""s)
                    .EndArray()
                .EndDict()
                .Build()
            },
            std::cout
        );
        std::cout << std::endl;

        json::Print(
            json::Document{
                json::Builder{}
                .Value("just a string"s)
                .Build()
            },
            std::cout
        );
        std::cout << std::endl;
    */

    /*
    cat::TransportCatalogue db;
    json::TransportCatalogueData tcd;
    svg::MapRenderer mr;

    RequestHandler request_handler{ db, tcd, mr };

    tcd.LoadRequests(db, std::cin);

    request_handler.JSONout(std::cout);
    */

    std::ifstream jsonfile1("C:\\CPP\\Yandex.Cpp\\Sprint_11\\testJSONin15-3.txt"s);
    if (jsonfile1.is_open()) {
        cat::TransportCatalogue db;
        json::TransportCatalogueData tcd;
        svg::MapRenderer mr;

        RequestHandler request_handler{ db, tcd, mr };

        tcd.LoadRequests(db, jsonfile1);
        jsonfile1.close();

        setlocale(LC_ALL, "Russian");

        std::cout << "TXT out:\n"s;
        request_handler.TXTout(std::cout);

        std::cout << "JSON out:\n"s;
        request_handler.JSONout(std::cout);

        std::ofstream outfile1("C:\\CPP\\Yandex.Cpp\\Sprint_11\\testJSONout15-3.svg"s);
        if (outfile1.is_open()) {
            request_handler.RenderMap(outfile1);
            outfile1.close();
        }
    }


    return 0;
}