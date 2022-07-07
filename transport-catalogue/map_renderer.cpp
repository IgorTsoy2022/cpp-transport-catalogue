#include "map_renderer.h"

namespace svg {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    Label::Label(const svg::Text text)
        : text_(text)
    {}

    void Label::Draw(svg::ObjectContainer& container) const {
        container.Add(text_);
    }


    std::map<std::string_view, svg::Point>
    StopCoords(const cat::TransportCatalogue& db,
        const svg::RouteMapSettings& route_map_settings,
        std::set<std::string_view>& sortered_bus_names) {

        std::map<std::string_view, svg::Point> coords;

        std::vector<geo::Coordinates> geo_coords;
        std::unordered_set<dom::STOP*> stops;
        for (const auto& [bus_name, bus] : db.GetBuses()) {
            for (const auto& stop : bus->stops) {
                sortered_bus_names.insert(bus_name);
                stops.insert(stop);
                geo_coords.push_back({ stop->latitude,
                                            stop->longitude });
            }
        }

        const svg::SphereProjector proj{
            geo_coords.begin(), geo_coords.end(),
            route_map_settings.width, route_map_settings.height,
            route_map_settings.padding
        };
        geo_coords.clear();

        for (const auto& stop : stops) {
            coords[stop->name]
                = proj({ stop->latitude, stop->longitude });
        }

        return coords;
    }

    svg::Polyline CreateRoute(const dom::BUS* bus,
        const std::map<std::string_view, svg::Point>& stop_coords,
        const svg::Color& fill_color,
        const svg::Color& stroke_color,
        const double stroke_width,
        const svg::StrokeLineCap stroke_line_cap,
        const svg::StrokeLineJoin stroke_line_join) {

        svg::Polyline polyline;

        for (const auto& stop : bus->stops) {
            polyline.AddPoint(stop_coords.at(stop->name));
        }

        if (!bus->is_annular) {
            for (auto it = bus->stops.rbegin() + 1;
                it != bus->stops.rend(); ++it) {
                polyline.AddPoint(stop_coords.at((*it)->name));
            }
        }

        polyline.SetFillColor(fill_color)
            .SetStrokeColor(stroke_color)
            .SetStrokeWidth(stroke_width)
            .SetStrokeLineCap(stroke_line_cap)
            .SetStrokeLineJoin(stroke_line_join);

        return polyline;
    }

    svg::Text BaseText(const std::string_view name,
        const svg::Point& coords,
        const svg::Point& offset,
        const int font_size,
        const std::string& font_family,
        const std::string& font_weight) {
        return svg::Text()
            .SetPosition(coords)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetFontWeight(font_weight)
            .SetData(std::string(name));
    }

    svg::Text BaseText(const std::string_view name,
        const svg::Point& coords,
        const svg::Point& offset,
        const int font_size,
        const std::string& font_family) {
        return svg::Text()
            .SetPosition(coords)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetData(std::string(name));
    }

    svg::Text Substrate(const svg::Text& base_text,
        const svg::Color& fill_color,
        const svg::Color& stroke_color,
        const double stroke_width) {
        return svg::Text{ base_text }
            .SetFillColor(fill_color)
            .SetStrokeColor(stroke_color)
            .SetStrokeWidth(stroke_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    svg::Text Caption(const svg::Text& base_text,
        const svg::Color& fill_color) {
        return svg::Text{ base_text }
        .SetFillColor(fill_color);
    }

    using namespace std::literals;

    svg::Document RenderMap(const cat::TransportCatalogue& db,
        const svg::RouteMapSettings& route_map_settings) {

        std::set<std::string_view> sortered_bus_names;

        std::map<std::string_view, svg::Point> stop_coords 
            = StopCoords(db, route_map_settings, sortered_bus_names);

        svg::Document doc;
        int colors_count
            = static_cast<int>(route_map_settings.color_palette.size());
        int color_index = 0;

        std::vector<std::unique_ptr<svg::Drawable>> text_container;
        const auto& buses = db.GetBuses();

        for (const auto& bus_name : sortered_bus_names) {

            const auto& bus = buses.at(bus_name);

            color_index = color_index >= colors_count ? 0 : color_index;

            svg::Polyline polyline = CreateRoute(bus,
                stop_coords,
                svg::NoneColor,
                route_map_settings.color_palette[color_index],
                route_map_settings.line_width,
                svg::StrokeLineCap::ROUND,
                svg::StrokeLineJoin::ROUND);

            doc.Add(std::move(polyline));

            auto base_text =
                BaseText(bus_name,
                    stop_coords.at(bus->stops.front()->name),
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
                        stop_coords.at(bus->stops.back()->name),
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

        for (const auto& [stop_name, coords] : stop_coords) {
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


} // namespace svg