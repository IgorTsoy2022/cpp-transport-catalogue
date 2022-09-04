#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <map>

namespace svg {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала
        // элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin,
            PointInputIt points_end,
            double max_width, double max_height,
            double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять
            // нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль
            // координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) /
                    (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль
            // координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) /
                    (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте
                // ненулевые, берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой,
                // используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой,
                // используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри
        // SVG-изображения
        dom::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    template <typename DrawableIterator>
    void DrawPicture(DrawableIterator begin, DrawableIterator end,
        svg::ObjectContainer& target) {
        for (auto it = begin; it != end; ++it) {
            (*it)->Draw(target);
        }
    }

    template <typename Container>
    void DrawPicture(const Container& container,
        svg::ObjectContainer& target) {
        DrawPicture(std::begin(container), std::end(container),
            target);
    }

    class Label : public svg::Drawable {
    public:
        Label(const svg::Text text);

        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Text text_;
    };

    class MapRenderer {
    public:

        const svg::Document RenderMap(const cat::TransportCatalogue& db);

        dom::RouteMapSettings& GetRouteMapSettings();

    private:
        dom::RouteMapSettings route_map_settings_;

        std::map<std::string_view, dom::Point>
        StopCoords(const cat::TransportCatalogue& db,
                   std::set<std::string_view>& sortered_bus_names);

        svg::Polyline CreateRoute(const dom::Bus* bus,
            const std::map<std::string_view, dom::Point>& stop_coords,
            const dom::Color& fill_color,
            const dom::Color& stroke_color,
            const double stroke_width,
            const svg::StrokeLineCap stroke_line_cap,
            const svg::StrokeLineJoin stroke_line_join);
        svg::Text BaseText(const std::string_view name,
            const dom::Point& coords,
            const dom::Point& offset,
            const int font_size,
            const std::string& font_family,
            const std::string& font_weight);
        svg::Text BaseText(const std::string_view name,
            const dom::Point& coords,
            const dom::Point& offset,
            const int font_size,
            const std::string& font_family);
        svg::Text Substrate(const svg::Text& base_text,
            const dom::Color& fill_color,
            const dom::Color& stroke_color,
            const double stroke_width);
        svg::Text Caption(const svg::Text& base_text,
            const dom::Color& fill_color);

    };

} // namespace svg