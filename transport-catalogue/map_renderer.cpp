#include "map_renderer.h"

namespace svg {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    dom::Point SphereProjector::operator()(geo::Coordinates coords) const {
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

    using namespace std::literals;

    const svg::Document MapRenderer::RenderMap(
        const cat::TransportCatalogue& db) {

        std::set<std::string_view> sortered_bus_names;
        std::map<std::string_view, dom::Point>
            stop_coords = StopCoords(db, sortered_bus_names);

        int colors_count =
            static_cast<int>(route_map_settings_.color_palette.size());
        int color_index = 0;

        std::vector<std::unique_ptr<svg::Drawable>> text_container;
        const auto& buses = db.GetBuses();

        svg::Document doc;
        for (const auto& bus_name : sortered_bus_names) {

            const auto& bus = buses.at(bus_name);

            color_index = color_index >= colors_count ? 0 : color_index;

            svg::Polyline polyline = CreateRoute(bus,
                stop_coords,
                svg::NoneColor,
                route_map_settings_.color_palette[color_index],
                route_map_settings_.line_width,
                svg::StrokeLineCap::ROUND,
                svg::StrokeLineJoin::ROUND);

            doc.Add(std::move(polyline));

            auto base_text =
                BaseText(bus_name,
                    stop_coords.at(bus->stops.front()->name),
                    route_map_settings_.bus_label_offset,
                    route_map_settings_.bus_label_font_size,
                    "Verdana"s, "bold"s);
            svg::Label substrate = Substrate(base_text,
                route_map_settings_.underlayer_color,
                route_map_settings_.underlayer_color,
                route_map_settings_.underlayer_width);
            svg::Label caption = Caption(base_text,
                route_map_settings_.color_palette[color_index]);
            text_container.push_back(
                std::make_unique<svg::Label>(substrate));
            text_container.push_back(
                std::make_unique<svg::Label>(caption));

            if (!bus->is_annular
                && bus->stops.front() != bus->stops.back()) {
                base_text =
                    BaseText(bus_name,
                        stop_coords.at(bus->stops.back()->name),
                        route_map_settings_.bus_label_offset,
                        route_map_settings_.bus_label_font_size,
                        "Verdana"s, "bold"s);
                svg::Label substrate = Substrate(base_text,
                    route_map_settings_.underlayer_color,
                    route_map_settings_.underlayer_color,
                    route_map_settings_.underlayer_width);
                svg::Label caption = Caption(base_text,
                    route_map_settings_.color_palette[color_index]);
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
                .SetRadius(route_map_settings_.stop_radius)
                .SetFillColor("white"s);

            doc.Add(std::move(circle));

            auto base_text = BaseText(stop_name, coords,
                route_map_settings_.stop_label_offset,
                route_map_settings_.stop_label_font_size, "Verdana"s);
            svg::Label substrate = Substrate(base_text,
                route_map_settings_.underlayer_color,
                route_map_settings_.underlayer_color,
                route_map_settings_.underlayer_width);
            svg::Label caption = Caption(base_text, "black"s);
            text_container.push_back(
                std::make_unique<svg::Label>(substrate));
            text_container.push_back(
                std::make_unique<svg::Label>(caption));
        }

        DrawPicture(text_container, doc);

        return doc;
    }

    dom::RouteMapSettings&
        MapRenderer::GetRouteMapSettings() {
        return route_map_settings_;
    }

    std::map<std::string_view, dom::Point>
    MapRenderer::StopCoords(const cat::TransportCatalogue& db,
                            std::set<std::string_view>& buses) {

        std::map<std::string_view, dom::Point> coords;

        std::vector<geo::Coordinates> geo_coords;
        std::unordered_set<dom::Stop*> stops;

        for (const auto& [bus_name, bus] : db.GetBuses()) {
            for (const auto& stop : bus->stops) {
                buses.insert(bus_name);
                stops.insert(stop);
                geo_coords.push_back({ stop->latitude,
                                       stop->longitude });
            }
        }

        const svg::SphereProjector proj{
            geo_coords.begin(), geo_coords.end(),
            route_map_settings_.width, route_map_settings_.height,
            route_map_settings_.padding
        };
        geo_coords.clear();

        for (const auto& stop : stops) {
            coords[stop->name]
                = proj({ stop->latitude, stop->longitude });
        }

        return coords;
    }

    svg::Polyline MapRenderer::CreateRoute(const dom::Bus* bus,
        const std::map<std::string_view, dom::Point>& stop_coords,
        const dom::Color& fill_color,
        const dom::Color& stroke_color,
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

    svg::Text MapRenderer::BaseText(const std::string_view name,
        const dom::Point& coords,
        const dom::Point& offset,
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

    svg::Text MapRenderer::BaseText(const std::string_view name,
        const dom::Point& coords,
        const dom::Point& offset,
        const int font_size,
        const std::string& font_family) {
        return svg::Text()
            .SetPosition(coords)
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily(font_family)
            .SetData(std::string(name));
    }

    svg::Text MapRenderer::Substrate(const svg::Text& base_text,
        const dom::Color& fill_color,
        const dom::Color& stroke_color,
        const double stroke_width) {
        return svg::Text{ base_text }
            .SetFillColor(fill_color)
            .SetStrokeColor(stroke_color)
            .SetStrokeWidth(stroke_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    svg::Text MapRenderer::Caption(const svg::Text& base_text,
        const dom::Color& fill_color) {
        return svg::Text{ base_text }
        .SetFillColor(fill_color);
    }

} // namespace svg