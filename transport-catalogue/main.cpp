#include "json_reader.h"
#include "request_handler.h"

int main() {

    cat::TransportCatalogue db;
    json::TransportCatalogueData tcd;
    RequestHandler request_handler{ db, tcd };

    tcd.LoadRequests(db, std::cin);
    request_handler.JSONout(std::cout);

    return 0;
}
