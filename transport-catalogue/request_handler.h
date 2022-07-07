#pragma once

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "svg.h"

#include <iomanip>
#include <iostream>
#include <optional>

 // ����� RequestHandler ������ ���� ������, �����������
 // �������������� JSON reader-� � ������� ������������
 // ����������.
 // ��. ������� �������������� �����:
 // https://ru.wikipedia.org/wiki/�����_(������_��������������)

 class RequestHandler {
 public:

     RequestHandler(const cat::TransportCatalogue& db,
         const json::TransportCatalogueData& loaded,
         svg::MapRenderer& map_renderer);

     void JSONout(std::ostream& out);
     void RenderMap(std::ostream& out);

/*
     // ���������� ��������, ���������� �����
     const std::unordered_set<BusPtr>*
     GetBusesByStop(const std::string_view& stop_name) const;
*/

 private:
     // RequestHandler ���������� ��������� ��������
     // "������������ ����������" � "������������ �����"
     const cat::TransportCatalogue& db_;
     const json::TransportCatalogueData& loaded_;
     svg::MapRenderer& map_renderer_;
 };
 
namespace cat {

    void TXTout(const cat::TransportCatalogue& db, 
        const std::vector<dom::Query>& requests,
        int precision = 9, std::ostream& out = std::cout);

    void BusInfo(const dom::BusInfo& bus_info,
        int precision = 9, std::ostream& out = std::cout);
    void StopInfo(const dom::StopInfo& stop_info,
        std::ostream& out = std::cout);

    void Stops(const TransportCatalogue& db,
        int precision = 9,
        std::ostream& out = std::cout);
    void Buses(const TransportCatalogue& db,
        std::ostream& out = std::cout);
    void Distances(const TransportCatalogue& db,
        std::ostream& out = std::cout);

} // namespace cat