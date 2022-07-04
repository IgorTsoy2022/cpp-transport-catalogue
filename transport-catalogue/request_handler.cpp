#include "request_handler.h"

RequestHandler::RequestHandler(const cat::TransportCatalogue& db,
    const json::TransportCatalogueData& loaded)
    : db_(db)
    , loaded_(loaded)
{}

using namespace std::literals;

svg::Document RequestHandler::RenderMap() {

    const auto& route_map_settings = loaded_.GetRouteMapSettings();

    SetStopMapCoords(route_map_settings.width, route_map_settings.height,
        route_map_settings.padding);

    svg::Document doc;
    int colors_count
        = static_cast<int>(route_map_settings.color_palette.size());
    int color_index = 0;

    std::vector<std::unique_ptr<svg::Drawable>> text_container;
    const auto& buses = db_.GetBuses();

    for (const auto& bus_name : route_names_) {

        const auto& bus = buses.at(bus_name);

        color_index = color_index >= colors_count ? 0 : color_index;

        svg::Polyline polyline = CreateRoute(bus,
            svg::NoneColor,
            route_map_settings.color_palette[color_index],
            route_map_settings.line_width,
            svg::StrokeLineCap::ROUND,
            svg::StrokeLineJoin::ROUND);

        doc.Add(std::move(polyline));

        auto base_text =
             BaseText(bus_name,
                 stop_map_coords_.at(bus->stops.front()->name),
                 route_map_settings.bus_label_offset,
                 route_map_settings.bus_label_font_size,
                 "Verdana"s, "bold"s);
        svg::Label substrate = Substrate(base_text,
            route_map_settings.underlayer_color,
            route_map_settings.underlayer_color,
            route_map_settings.underlayer_width);
        svg::Label caption = Caption(base_text,
            route_map_settings.color_palette[color_index]);
        text_container.push_back(
            std::make_unique<svg::Label>(substrate));
        text_container.push_back(
            std::make_unique<svg::Label>(caption));

        if (!bus->is_annular 
            && bus->stops.front() != bus->stops.back()) {
            base_text = 
                BaseText(bus_name,
                    stop_map_coords_.at(bus->stops.back()->name),
                    route_map_settings.bus_label_offset,
                    route_map_settings.bus_label_font_size,
                    "Verdana"s, "bold"s);
            svg::Label substrate = Substrate(base_text,
                route_map_settings.underlayer_color,
                route_map_settings.underlayer_color,
                route_map_settings.underlayer_width);
            svg::Label caption = Caption(base_text,
                route_map_settings.color_palette[color_index]);
            text_container.push_back(
                std::make_unique<svg::Label>(substrate));
            text_container.push_back(
                std::make_unique<svg::Label>(caption));
        }

        ++color_index;
    }

    DrawPicture(text_container, doc);
    text_container.clear();

    for (const auto& [stop_name, coords] : stop_map_coords_) {
        svg::Circle circle;
        circle.SetCenter(coords)
            .SetRadius(route_map_settings.stop_radius)
            .SetFillColor("white"s);

        doc.Add(std::move(circle));

        auto base_text = BaseText(stop_name, coords,
            route_map_settings.stop_label_offset,
            route_map_settings.stop_label_font_size, "Verdana"s);
        svg::Label substrate = Substrate(base_text,
            route_map_settings.underlayer_color,
            route_map_settings.underlayer_color,
            route_map_settings.underlayer_width);
        svg::Label caption = Caption(base_text, "black"s);
        text_container.push_back(
            std::make_unique<svg::Label>(substrate));
        text_container.push_back(
            std::make_unique<svg::Label>(caption));
    }

    DrawPicture(text_container, doc);

    return doc;
}

const std::vector<dom::QUERY>& RequestHandler::GetRequests() const {
    return loaded_.GetRequests();
}

void RequestHandler::JSONout(std::ostream& out) {

    json::Array root;

    for (const auto& request : loaded_.GetRequests()) {
        json::Dict blocks;
        blocks["request_id"s] =
            std::move(json::Node(request.id));

        if (request.type == dom::QueryType::STOP) {
            auto stop_info = db_.GetStopInfo(request.name);
            if (stop_info.exists) {
                json::Array items;
                if (stop_info.buses.size() > 0) {
                    for (auto bus : stop_info.buses) {
                        items.push_back(
                            std::move(json::Node(
                                bus->name)));
                    }
                }
                blocks["buses"s] =
                    std::move(json::Node(std::move(
                        items)));
            }
            else {
                blocks["error_message"s] =
                    std::move(json::Node(
                        "not found"s));
            }
        }
        else if (request.type == dom::QueryType::BUS) {
            auto bus_info = db_.GetBusInfo(request.name);
            if (bus_info.route_stops > 0) {
                blocks["stop_count"s] =
                    std::move(json::Node(
                        bus_info.route_stops));
                blocks["unique_stop_count"s] =
                    std::move(json::Node(
                        bus_info.unique_stops));
                blocks["route_length"s] =
                    std::move(json::Node(bus_info.length));
                blocks["curvature"s] =
                    std::move(json::Node(bus_info.curvature));
            }
            else {
                blocks["error_message"s] =
                    std::move(json::Node("not found"s));
            }
        }
        else if (request.type == dom::QueryType::MAP) {
            std::ostringstream map_stream;
            RenderMap().Render(map_stream);
            blocks["map"s] = std::move(json::Node(map_stream.str()));
        }

        root.push_back(std::move(json::Node(std::move(
            blocks))));
    }

    json::Document doc{ std::move(json::Node(root)) };
    json::Print(doc, out);
}



void RequestHandler::SetStopMapCoords(double width, double height, double padding) {

    std::vector<geo::Coordinates> stop_geo_coords;
    std::unordered_set<dom::STOP*> stops;
    for (const auto& [bus_name, bus] : db_.GetBuses()) {
        for (const auto& stop : bus->stops) {
            route_names_.insert(bus_name);
            stops.insert(stop);
            stop_geo_coords.push_back({ stop->latitude,
                                        stop->longitude });
        }
    }

    const svg::SphereProjector proj{
        stop_geo_coords.begin(), stop_geo_coords.end(),
        width, height, padding
    };
    stop_geo_coords.clear();

    for (const auto& stop : stops) {
        stop_map_coords_[stop->name]
            = proj({ stop->latitude, stop->longitude });
    }
}

svg::Polyline RequestHandler::CreateRoute(const dom::BUS* bus,
    const svg::Color& fill_color,
    const svg::Color& stroke_color,
    const double stroke_width,
    const svg::StrokeLineCap stroke_line_cap,
    const svg::StrokeLineJoin stroke_line_join) const {

    svg::Polyline polyline;

    for (const auto& stop : bus->stops) {
        polyline.AddPoint(stop_map_coords_.at(stop->name));
    }

    if (!bus->is_annular) {
        for (auto it = bus->stops.rbegin() + 1;
            it != bus->stops.rend(); ++it) {
            polyline.AddPoint(stop_map_coords_.at((*it)->name));
        }
    }

    polyline.SetFillColor(fill_color)
        .SetStrokeColor(stroke_color)
        .SetStrokeWidth(stroke_width)
        .SetStrokeLineCap(stroke_line_cap)
        .SetStrokeLineJoin(stroke_line_join);

    return polyline;
}

svg::Text RequestHandler::BaseText(const std::string_view name, 
    const svg::Point& coords,
    const svg::Point& offset,
    const int font_size,
    const std::string& font_family,
    const std::string& font_weight) const {
    return svg::Text()
        .SetPosition(coords)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily(font_family)
        .SetFontWeight(font_weight)
        .SetData(std::string(name));
}

svg::Text RequestHandler::BaseText(const std::string_view name,
    const svg::Point& coords,
    const svg::Point& offset,
    const int font_size,
    const std::string& font_family) const {
    return svg::Text()
        .SetPosition(coords)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily(font_family)
        .SetData(std::string(name));
}

svg::Text RequestHandler::Substrate(const svg::Text& base_text,
    const svg::Color& fill_color,
    const svg::Color& stroke_color,
    const double stroke_width) const {
    return svg::Text{ base_text }
        .SetFillColor(fill_color)
        .SetStrokeColor(stroke_color)
        .SetStrokeWidth(stroke_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

svg::Text RequestHandler::Caption(const svg::Text& base_text,
    const svg::Color& fill_color) const {
    return svg::Text{ base_text }
        .SetFillColor(fill_color);
}


/*
const std::unordered_map<std::string_view, dom::BUS*>&
RequestHandler::GetBuses() const {
    return db_.GetBuses();
}

const svg::RouteMapSettings&
RequestHandler::GetMapRouteSettings() const {
    return route_map_settings_;
}
*/

namespace cat {

    void TXTout(const cat::TransportCatalogue& db,
        const std::vector<dom::QUERY>& requests,
        int precision, std::ostream& out) {
        for (const auto& request : requests) {
            if (request.type == dom::QueryType::BUS) {
                auto bus_info = db.GetBusInfo(request.name);
                BusInfo(bus_info, precision, out);
                continue;
            }
            if (request.type == dom::QueryType::STOP) {
                auto stop_info = db.GetStopInfo(request.name);
                StopInfo(stop_info, out);
                continue;
            }
            out << "Unknown request."sv << std::endl;
        }
    }

    void BusInfo(const dom::BUSinfo& bus_info, int precision,
        std::ostream& out) {
        out << std::setprecision(precision)
            << "Bus "sv << bus_info.name << ": "sv;
        if (bus_info.route_stops > 0) {
            out << bus_info.route_stops << " stops on route, "sv
                << bus_info.unique_stops << " unique stops, "sv
                << bus_info.length << " route length, "sv
                << bus_info.curvature << " curvature"sv;
        }
        else {
            out << "not found"sv;
        }
        out << std::endl;
    }

    void StopInfo(const dom::STOPinfo& stop_info, std::ostream& out) {
        out << "Stop "sv << stop_info.name << ": "sv;
        if (stop_info.exists) {
            if (stop_info.buses.size() > 0) {
                out << "buses"sv;
                for (const auto& bus : stop_info.buses) {
                    out << " "sv << bus->name;
                }
            }
            else {
               out << "no buses"sv;
            }
        }
        else {
            out << "not found"sv;
        }
        out << std::endl;
    }

    void Stops(const TransportCatalogue& db, int precision,
        std::ostream& out) {
        for (auto& [key, coords] : db.GetStops()) {
            out << std::setprecision(precision)
                << "Stop ["sv << key
                << "]: "sv << coords->latitude
                << ", "sv << coords->longitude << std::endl;
        }
    }

    void Buses(const TransportCatalogue& db,
        std::ostream& out) {
        for (auto& [key, bus] : db.GetBuses()) {
            out << "Bus ["sv << key << "]: "sv;
            auto count = bus->stops.size();
            for (auto& stop : bus->stops) {
                out << "["sv << stop->name << "]"sv
                    << ((--count > 0) ? " > "sv : ""sv);
            }
            out << std::endl;
        }
    }

    void Distances(const TransportCatalogue& db,
        std::ostream& out) {
        for (auto& [key, distance] : db.GetDistances()) {
            out << "Distance between ["sv << key.first->name
                << "] - ["sv << key.second->name << "] = "sv
                << distance << std::endl;
        }
    }

} // namespace cat