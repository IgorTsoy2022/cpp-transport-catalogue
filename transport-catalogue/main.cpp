#include "request_handler.h"

#include <fstream>
#include <iostream>

using namespace std::string_literals;

int main() {

    cat::TransportCatalogue db;
    json::Reader rdr;
    svg::MapRenderer mr;

    RequestHandler request_handler{ db, rdr, mr };
    rdr.LoadRequests(db, std::cin);
    request_handler.JSONout(std::cout);

/*
    std::ifstream jsonfile("C:\\CPP\\Yandex.Cpp\\Sprint_12\\TC_test3-in.txt"s);
    if (jsonfile.is_open()) {
        cat::TransportCatalogue db;
        json::Reader rdr;
        svg::MapRenderer mr;

        RequestHandler request_handler{ db, rdr, mr };

        rdr.LoadRequests(db, jsonfile);
        jsonfile.close();

        setlocale(LC_ALL, "Russian");

        std::cout << "TXT out:\n"s;
        request_handler.TXTout(std::cout);

        std::cout << "JSON out:\n"s;
        std::ofstream outfile("C:\\CPP\\Yandex.Cpp\\Sprint_12\\TC_test3-out.txt"s);
        if (outfile.is_open()) {
            request_handler.JSONout(outfile);
            outfile.close();
        }

        std::ofstream outfile1("C:\\CPP\\Yandex.Cpp\\Sprint_12\\TC_test3-out.svg"s);
        if (outfile1.is_open()) {
            request_handler.RenderMap(outfile1);
            outfile1.close();
        }
    }
*/
    return 0;
}