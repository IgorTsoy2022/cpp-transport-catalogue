#pragma once

#include "json.h"
#include "json_builder.h"
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
    void TXTout(std::ostream& out, int precision = 6);
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

    void BusInfo(const dom::BusInfo& bus_info, int precision,
        std::ostream& out);
    void StopInfo(const dom::StopInfo& stop_info,
        std::ostream& out);
};