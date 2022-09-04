#include "serialization.h"

using namespace std::literals;

namespace serialization {

    //------------------------- Seriliazation -------------------------//

    void Portal::Serialize(const Path& file,
                           cat::TransportCatalogue& db,
                           svg::MapRenderer& map_renderer,
                           cat::TransportRouter& transport_router) const {

        std::ofstream out(file, std::ios::binary);
        cat_proto::TransportCatalogueBase destination;

//        std::unordered_map<std::string_view, size_t>& 
//        stops_ids = router.GetStopsIds();
//        if (!db.RouterIsSet()) {
//            router.BuildGraph(db);
//            db.SetRouterIsSet(true);
//        }
 
        const std::unordered_map<std::string_view, dom::Stop*>&
        stops = db.GetStops();
        const cat::MapStopsDistance&
        distances = db.GetDistances();
        const std::unordered_map<std::string_view, dom::Bus*>&
        buses = db.GetBuses();

        for (const auto& [_, stop] : stops) {
            cat_proto::Stop* stop_proto = destination.add_stops();
            stop_proto->set_id(reinterpret_cast<size_t>(stop));
            stop_proto->set_name(stop->name);
            stop_proto->set_latitude(stop->latitude);
            stop_proto->set_longitude(stop->longitude);

        //    if (send_router) {
        //        stop_proto->set_id(transport_router.GetStopsIds().at(stop->name));
        //    }
        //    else {
        //        stop_proto->set_id(reinterpret_cast<size_t>(stop));
        //    }
        }


        for (const auto& [key, value] : distances) {
            cat_proto::Distance* distance_proto = 
                destination.add_road_distances();

//            auto i = transport_router.GetStopsIds().at(key.first->name);

            const auto from_stop =
                reinterpret_cast<size_t>(key.first);
            const auto to_stop =
                reinterpret_cast<size_t>(key.second);

            distance_proto->set_from_stop_id(from_stop);
            distance_proto->set_to_stop_id(to_stop);
            distance_proto->set_distance(value);
        }

        for (const auto& [_, bus] : buses) {
            cat_proto::Bus* bus_proto = destination.add_buses();
            bus_proto->set_name(bus->name);
            bus_proto->set_is_annular(bus->is_annular);
            for (const auto& stop : bus->stops) {
                bus_proto->add_stop_ids(reinterpret_cast<size_t>(stop));
            }
        }

        *destination.mutable_route_map_settings() = 
            ConvertToProto(map_renderer.GetRouteMapSettings());

        *destination.mutable_routing_settings() = 
            ConvertToProto(transport_router.GetRoutingSettings());

        destination.SerializeToOstream(&out);
    }

    cat_proto::RouteMapSettings ConvertToProto(
        const dom::RouteMapSettings& route_map_settings) {

        cat_proto::RouteMapSettings route_map_settings_proto;

        route_map_settings_proto.set_width(route_map_settings.width);
        route_map_settings_proto.set_height(route_map_settings.height);
        route_map_settings_proto.set_padding(route_map_settings.padding);
        route_map_settings_proto.
            set_line_width(route_map_settings.line_width);
        route_map_settings_proto.
            set_stop_radius(route_map_settings.stop_radius);

        route_map_settings_proto.
            set_bus_label_font_size(route_map_settings.bus_label_font_size);

        svg_proto::Point bus_offset_proto;
        bus_offset_proto.set_x(route_map_settings.bus_label_offset.x);
        bus_offset_proto.set_y(route_map_settings.bus_label_offset.y);
        *route_map_settings_proto.mutable_bus_label_offset() = bus_offset_proto;

        route_map_settings_proto.
            set_stop_label_font_size(route_map_settings.stop_label_font_size);

        svg_proto::Point stop_offset_proto;
        stop_offset_proto.set_x(route_map_settings.stop_label_offset.x);
        stop_offset_proto.set_y(route_map_settings.stop_label_offset.y);
        *route_map_settings_proto.mutable_stop_label_offset() = stop_offset_proto;

        svg_proto::Color color_proto =
            ConvertToProto(route_map_settings.underlayer_color);
        *route_map_settings_proto.mutable_underlayer_color() = color_proto;

        route_map_settings_proto.
            set_underlayer_width(route_map_settings.underlayer_width);

        for (const auto& color : route_map_settings.color_palette) {
            svg_proto::Color* color_proto =
                route_map_settings_proto.add_color_palette();
            *color_proto = ConvertToProto(color);
        }

        return route_map_settings_proto;
    }

    svg_proto::Color ConvertToProto(const dom::Color& color) {

        svg_proto::Color color_proto;

        const auto& color_type = GetVariantType<dom::Color>(color);

        if (color_type == typeid(std::string)) {
            std::string string_value = std::get<std::string>(color);
            color_proto.set_string_value(string_value);
            return color_proto;
        }

        if (color_type == typeid(dom::Rgb)) {
            dom::Rgb rgb_value = std::get<dom::Rgb>(color);
            svg_proto::RGB rgb_proto;
            rgb_proto.set_red(rgb_value.red);
            rgb_proto.set_green(rgb_value.green);
            rgb_proto.set_blue(rgb_value.blue);
            *color_proto.mutable_rgb_value() = rgb_proto;
            return color_proto;
        }

        if (color_type == typeid(dom::Rgba)) {
            dom::Rgba rgba_value = std::get<dom::Rgba>(color);
            svg_proto::RGBa rgba_proto;
            rgba_proto.set_red(rgba_value.red);
            rgba_proto.set_green(rgba_value.green);
            rgba_proto.set_blue(rgba_value.blue);
            rgba_proto.set_opacity(rgba_value.opacity);
            *color_proto.mutable_rgba_value() = rgba_proto;
            return color_proto;
        }

        return color_proto;
    }

    cat_proto::RoutingSettings ConvertToProto(
        const dom::RoutingSettings& routing_settings) {

        cat_proto::RoutingSettings routing_settings_proto;
        routing_settings_proto.set_bus_velocity(routing_settings.bus_velocity);
        routing_settings_proto.set_bus_wait_time(routing_settings.bus_wait_time);

        return routing_settings_proto;
    }

    //------------------------ Deseriliazation ------------------------//

    void Portal::Deserialize(const Path& file,
        cat::TransportCatalogue& db,
        svg::MapRenderer& map_renderer,
        cat::TransportRouter& transport_router) const {

        std::ifstream in(file, std::ios::binary);
        cat_proto::TransportCatalogueBase source;

        std::unordered_map<size_t, std::string> stops;

        source.ParseFromIstream(&in);

//        auto& router_stops = transport_router.GetStops();
//        auto& router_stops_ids = transport_router.GetStopsIds();
//        router_stops.resize(source.stops_size());

        for (const auto& stop : source.stops()) {
            db.AddStop({ stop.name(), stop.latitude(), stop.longitude() });
            stops[stop.id()] = stop.name();

//            router_stops[stop.id()] = db.GetStop(stop.name());
//            router.GetStops();
//            router.GetStopsIds();
        }

        std::vector<cat::StopsDistance> stops_distances;
        stops_distances.reserve(source.road_distances_size());

        for (const auto& distance : source.road_distances()) {
            if (stops.count(distance.from_stop_id()) == 0 ||
                stops.count(distance.to_stop_id()) == 0) {
                throw std::invalid_argument(
                    "Invalid Stop in Distances"s);
            }

            cat::StopsDistance s { stops.at(distance.from_stop_id()),
                                   stops.at(distance.to_stop_id()),
                                   distance.distance() };
            
            stops_distances.emplace_back(std::move(s));
        }

        db.AddStopDistances(std::move(stops_distances));

        for (const auto& bus : source.buses()) {
            std::vector<std::string> bus_stops;
            bus_stops.reserve(bus.stop_ids_size());
            for (const auto& id : bus.stop_ids()) {
                if (stops.count(id) == 0) {
                    throw std::invalid_argument(
                        "Invalid Stop in Buses"s);
                }
                bus_stops.emplace_back(stops.at(id));
            }
            db.AddBus(bus.name(), bus.is_annular(), std::move(bus_stops));
        }

        if (source.has_route_map_settings()) {
            map_renderer.GetRouteMapSettings() =
                RestoreFromProto(source.route_map_settings());
        }

        if (source.has_routing_settings()) {
            transport_router.GetRoutingSettings() =
                RestoreFromProto(source.routing_settings());
        }
    }

    dom::RouteMapSettings RestoreFromProto(
        const cat_proto::RouteMapSettings& route_map_settings_proto) {

        dom::RouteMapSettings route_map_settings;

        route_map_settings.width =
            route_map_settings_proto.width();
        route_map_settings.height =
            route_map_settings_proto.height();
        route_map_settings.padding =
            route_map_settings_proto.padding();
        route_map_settings.line_width =
            route_map_settings_proto.line_width();
        route_map_settings.stop_radius =
            route_map_settings_proto.stop_radius();

        route_map_settings.bus_label_font_size =
            route_map_settings_proto.bus_label_font_size();
        route_map_settings.bus_label_offset.x =
            route_map_settings_proto.bus_label_offset().x();
        route_map_settings.bus_label_offset.y =
            route_map_settings_proto.bus_label_offset().y();

        route_map_settings.stop_label_font_size =
            route_map_settings_proto.stop_label_font_size();
        route_map_settings.stop_label_offset.x =
            route_map_settings_proto.stop_label_offset().x();
        route_map_settings.stop_label_offset.y =
            route_map_settings_proto.stop_label_offset().y();

        route_map_settings.underlayer_color = 
            RestoreFromProto(route_map_settings_proto.underlayer_color());

        route_map_settings.underlayer_width =
            route_map_settings_proto.underlayer_width();

        auto size = route_map_settings_proto.color_palette_size();
        route_map_settings.color_palette.resize(size);
        size_t i = 0;
        for (const auto& color :
            route_map_settings_proto.color_palette()) {
            route_map_settings.color_palette[i++] = 
                RestoreFromProto(color);
        }

        return route_map_settings;
    }

    dom::Color RestoreFromProto(const svg_proto::Color& color_proto) {

        dom::Color color;

        if (color_proto.has_string_value()) {
            return color_proto.string_value();
        }

        if (color_proto.has_rgb_value()) {
            svg_proto::RGB rgb_value;
            rgb_value = color_proto.rgb_value();
            return dom::Rgb { static_cast<uint8_t>(rgb_value.red()),
                              static_cast<uint8_t>(rgb_value.green()),
                              static_cast<uint8_t>(rgb_value.blue()) };
        }

        if (color_proto.has_rgba_value()) {
            svg_proto::RGBa rgba_value;
            rgba_value = color_proto.rgba_value();
            return dom::Rgba { static_cast<uint8_t>(rgba_value.red()),
                               static_cast<uint8_t>(rgba_value.green()),
                               static_cast<uint8_t>(rgba_value.blue()),
                               rgba_value.opacity() };
        }

        return {};
    }

    dom::RoutingSettings RestoreFromProto(
        const cat_proto::RoutingSettings& routing_settings_proto) {

        dom::RoutingSettings routing_settings;

        routing_settings.bus_velocity =
            routing_settings_proto.bus_velocity();
        routing_settings.bus_wait_time =
            routing_settings_proto.bus_wait_time();

        return routing_settings;
    }

    dom::SerializationSettings&
        Portal::GetSerializationSettings() {
        return serialization_settings_;
    }

} // namespace serialization