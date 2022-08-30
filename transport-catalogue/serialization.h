#pragma once

#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"

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
            const cat::TransportCatalogue& db) const;

        void Deserialize(const Path& file, cat::TransportCatalogue& db);

    private:
    };

    template<class V>
    std::type_info const& GetVariantType(V const& value) {
        return std::visit([](auto&& x)->decltype(auto) {
                             return typeid(x);
                            }, value);
    }

    cat_serialize::RouteMapSettings ConvertToProto(
        const dom::RouteMapSettings& route_map_settings);

    cat_serialize::Color ConvertToProto(const dom::Color& color);

    cat_serialize::RoutingSettings ConvertToProto(
        const dom::RoutingSettings& routing_settings);


    dom::RouteMapSettings RestoreFromProto(
        const cat_serialize::RouteMapSettings& route_map_settings_proto);

    dom::Color RestoreFromProto(const cat_serialize::Color& color_proto);

    dom::RoutingSettings RestoreFromProto(
        const cat_serialize::RoutingSettings& routing_settings_proto);

} // namespace serialization