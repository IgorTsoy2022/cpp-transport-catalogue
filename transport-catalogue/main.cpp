#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <fstream>

using namespace std::string_literals;
using namespace std::string_view_literals;

int main() {

    cat::TransportCatalogue tc;
    auto queries = cat::queries::QueriesToDataBase(tc);   
    cat::print::Info(tc, queries, 6);

    return 0;
}