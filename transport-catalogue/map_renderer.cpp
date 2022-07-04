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

} // namespace svg