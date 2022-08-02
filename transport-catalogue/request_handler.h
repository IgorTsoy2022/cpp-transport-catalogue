#pragma once

#include "json_builder.h"
#include "json_reader.h"

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