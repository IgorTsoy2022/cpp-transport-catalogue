#pragma once

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"

#include <iomanip>
#include <iostream>
#include <optional>

 // Класс RequestHandler играет роль Фасада, упрощающего
 // взаимодействие JSON reader-а с другими подсистемами
 // приложения.
 // См. паттерн проектирования Фасад:
 // https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

 class RequestHandler {
 public:

     RequestHandler(const cat::TransportCatalogue& db,
         const json::TransportCatalogueData& loaded);

     void JSONout(std::ostream& out);

/*
     // Возвращает маршруты, проходящие через
     const std::unordered_set<BusPtr>*
     GetBusesByStop(const std::string_view& stop_name) const;
*/

 private:
     // RequestHandler использует агрегацию объектов
     // "Транспортный Справочник" и "Визуализатор Карты"
     const cat::TransportCatalogue& db_;
     const json::TransportCatalogueData& loaded_;
 };
 
namespace cat {

    void TXTout(const cat::TransportCatalogue& db, 
        const std::vector<dom::QUERY>& requests,
        int precision = 9, std::ostream& out = std::cout);

    void BusInfo(const dom::BUSinfo& bus_info,
        int precision = 9, std::ostream& out = std::cout);
    void StopInfo(const dom::STOPinfo& stop_info,
        std::ostream& out = std::cout);

    void Stops(const TransportCatalogue& db,
        int precision = 9,
        std::ostream& out = std::cout);
    void Buses(const TransportCatalogue& db,
        std::ostream& out = std::cout);
    void Distances(const TransportCatalogue& db,
        std::ostream& out = std::cout);

} // namespace cat