#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>

namespace cat {
namespace print {

void Info(TransportCatalogue& db, const std::vector<QUERY>& queries,
          int precision = 9, std::ostream& out = std::cout);

void BusInfo(const BUS_Info& bus, int precision = 9,
             std::ostream& out = std::cout);
void StopInfo(const STOP_Info& stop,
              std::ostream& out = std::cout);

void Stops(const TransportCatalogue& tc, int precision = 9,
           std::ostream& out = std::cout);
void Buses(const TransportCatalogue& tc,
           std::ostream& out = std::cout);
void Distances(const TransportCatalogue& tc,
               std::ostream& out = std::cout);

} // namespace print
} // namespace cat