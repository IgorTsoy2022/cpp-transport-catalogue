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

    RequestHandler(json::Reader& reader,
        cat::TransportCatalogue& db,
        svg::MapRenderer& map_renderer,
        cat::TransportRouter& transport_router);

/*
    json::Reader& GetReader();
    cat::TransportCatalogue& GetTransportCatalogue();
    svg::MapRenderer& GetMapRenderer();
    cat::TransportRouter& GetTransportRouter();
*/

    const void BuildRouter() const;

    const void JSONout(std::ostream& out) const;
    const void TXTout(std::ostream& out, int precision = 6) const;
    const void RenderMap(std::ostream& out) const;

private:
    json::Reader& reader_;
    cat::TransportCatalogue& db_;
    svg::MapRenderer& map_renderer_;
    cat::TransportRouter& transport_router_;

    const void StopInfo(const dom::Query& request,
        json::Dict& blocks) const;
    const void BusInfo(const dom::Query& request,
        json::Dict& blocks) const;
    const void RouterInfo(const dom::Query& request,
        json::Dict& blocks) const;

    const void StopInfo(const dom::Query& request,
        std::ostream& out) const;
    const void BusInfo(const dom::Query& request, int precision,
        std::ostream& out) const;
    const void RouterInfo(const dom::Query& request,
        std::ostream& out) const;
};