#include "svg.h"

namespace svg {

    // ------------------------- Colour -------------------------

    std::ostream& operator<<(std::ostream& out,
        const svg::Color& value) {
        std::ostringstream strm;
        std::visit(svg::ColorOutputStream{ strm }, value);
        out << strm.str();
        return out;
    }

    std::ostream& operator<<(std::ostream& out,
        const svg::StrokeLineCap& value) {
        out << LINECAP[static_cast<int>(value)];
        return out;
    }

    std::ostream& operator<<(std::ostream& out,
        const svg::StrokeLineJoin& value) {
        out << LINEJOIN[static_cast<int>(value)];
        return out;
    }

    // ------------------------- Object -------------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // ƒелегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ------------------------- Circle -------------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(
        const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x
            << "\" cy=\""sv << center_.y << "\""sv
            << " r=\""sv << radius_ << "\""sv;
        // ¬ыводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ------------------------ Polyline ------------------------

    Polyline& Polyline::AddPoint(Point new_point) {
        points_.push_back(new_point);
        return *this;
    }

    void Polyline::RenderObject(
        const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first_point = true;
        for (const auto& point : points_) {
            if (first_point) {
                out << point.x << ","sv << point.y;
                first_point = false;
                continue;
            }
            out << " "sv << point.x << ","sv << point.y;
        }
        out << "\""sv;
        // ¬ыводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // -------------------------- Text --------------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(
        const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv
            << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y
            << "\""sv
            << " dx=\""sv << offset_.x << "\" dy=\""sv
            << offset_.y
            << "\""sv
            << " font-size=\""sv << size_
            << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_
                << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_
                << "\""sv;
        }
        // ¬ыводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << ">"sv;
        for (const auto& c : data_) {
            switch (c) {
            case '\"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out << c;
            }
        }
        out << "</text>"sv;
    }

    // ------------------------ Document ------------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv
            << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv
            << std::endl;

        for (const auto& object : objects_) {
            (*object).Render(out);
        }

        out << "</svg>\n"sv << std::endl;
    }

}  // namespace svg