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

     svg::Document RenderMap();

     const std::vector<dom::QUERY>& GetRequests() const;

     void JSONout(std::ostream& out);

 //    const std::unordered_map<std::string_view, dom::BUS*>&
 //    GetBuses() const;

 //    const svg::RouteMapSettings& GetMapRouteSettings() const;

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

     std::set<std::string_view> route_names_;
     std::map<std::string_view, svg::Point>
         stop_map_coords_;

     void SetStopMapCoords(double width, double height,
         double padding);

     svg::Polyline CreateRoute(const dom::BUS* bus,
         const svg::Color& fill_color,
         const svg::Color& stroke_color,
         const double stroke_width,
         const svg::StrokeLineCap stroke_line_cap,
         const svg::StrokeLineJoin stroke_line_join) const;

     svg::Text BaseText(const std::string_view name,
         const svg::Point& coords,
         const svg::Point& offset,
         const int font_size,
         const std::string& font_family,
         const std::string& font_weight) const;
     svg::Text BaseText(const std::string_view name,
         const svg::Point& coords,
         const svg::Point& offset,
         const int font_size,
         const std::string& font_family) const;

     svg::Text Substrate(const svg::Text& base_text,
         const svg::Color& fill_color,
         const svg::Color& stroke_color,
         const double stroke_width) const;
     svg::Text Caption(const svg::Text& base_text,
                       const svg::Color& fill_color) const;
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