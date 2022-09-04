#pragma once

#include <transport_catalogue.pb.h>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <typeinfo>

namespace serialization {

    using Path = std::filesystem::path;

    class Portal {
    public:
        Portal() = default;

        void Serialize(const Path& file,
                       cat::TransportCatalogue& db,
                       svg::MapRenderer& map_renderer,
                       cat::TransportRouter& transport_router) const;

        void Deserialize(const Path& file,
                         cat::TransportCatalogue& db,
                         svg::MapRenderer& map_renderer,
                         cat::TransportRouter& transport_router) const;

        dom::SerializationSettings& GetSerializationSettings();

    private:
        dom::SerializationSettings serialization_settings_;
    };

    template<class V>
    std::type_info const& GetVariantType(V const& value) {
        return std::visit([](auto&& x)->decltype(auto) {
                             return typeid(x);
                            }, value);
    }

    cat_proto::RouteMapSettings ConvertToProto(
        const dom::RouteMapSettings& route_map_settings);

    svg_proto::Color ConvertToProto(const dom::Color& color);

    cat_proto::RoutingSettings ConvertToProto(
        const dom::RoutingSettings& routing_settings);


    dom::RouteMapSettings RestoreFromProto(
        const cat_proto::RouteMapSettings& route_map_settings_proto);

    dom::Color RestoreFromProto(const svg_proto::Color& color_proto);

    dom::RoutingSettings RestoreFromProto(
        const cat_proto::RoutingSettings& routing_settings_proto);

} // namespace serialization