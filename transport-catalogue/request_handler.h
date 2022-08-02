#pragma once

#include "json_builder.h"
#include "json_reader.h"

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
        json::Reader& loaded,
        svg::MapRenderer& map_renderer);

    void JSONout(std::ostream& out);
    void TXTout(std::ostream& out, int precision = 6);
    void RenderMap(std::ostream& out);

private:
    const cat::TransportCatalogue& db_;
    json::Reader& loaded_;
    svg::MapRenderer& map_renderer_;

    cat::TransportRouter transport_router_;

    void BusInfo(const dom::BusInfo& bus_info, int precision,
        std::ostream& out);
    void StopInfo(const dom::StopInfo& stop_info,
        std::ostream& out);
};