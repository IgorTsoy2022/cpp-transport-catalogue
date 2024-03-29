#pragma once

#include "domain.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    using namespace std::literals;

    // ��������������� ���������, �������� �������� ��� ������
    // SVG-��������� � ���������.
    // ������ ������ �� ����� ������, ������� �������� � ���
    // ������� ��� ������ �������� 
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out)
        {}

        RenderContext(std::ostream& out, int indent_step,
            int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent)
        {}

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    // ------------------------- Colour -------------------------

    struct ColorOutputStream {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none"sv;
        }

        void operator()(const std::string value) const {
            out << value;
        }

        void operator()(const dom::Rgb& value) const {
            out << "rgb("sv << +value.red << ","sv
                << +value.green << ","sv
                << +value.blue << ")"sv;
        }

        void operator()(const dom::Rgba& value) const {
            out << "rgba("sv << +value.red << ","sv
                << +value.green << ","sv
                << +value.blue << ","sv
                << +value.opacity << ")"sv;
        }
    };

    std::ostream& operator<<(std::ostream& out,
        const dom::Color& value);

    // ������� � ������������ ����� ��������� �� ��������������
    // inline, �� ������� ���, ��� ��� ����� ����� �� ��� �������
    // ����������, ������� ���������� ���� ���������.
    // � ��������� ������ ������ ������� ���������� �����
    // ������������ ���� ����� ���� ���������
    inline const dom::Color NoneColor = std::monostate{};

    inline const std::string_view LINECAP[3] =
    { "butt"sv, "round"sv, "square"sv };
    inline const std::string_view LINEJOIN[5] =
    { "arcs"sv, "bevel"sv, "miter"sv, "miter-clip"sv, "round"sv };

    enum class StrokeLineCap {
        BUTT, ROUND, SQUARE
    };

    enum class StrokeLineJoin {
        ARCS, BEVEL, MITER, MITER_CLIP, ROUND
    };

    std::ostream& operator<<(std::ostream& out,
        const StrokeLineCap& data);
    std::ostream& operator<<(std::ostream& out,
        const StrokeLineJoin& data);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(dom::Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(dom::Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap linecap) {
            stroke_linecap_ = std::move(linecap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin linejoin) {
            stroke_linejoin_ = std::move(linejoin);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            if (fill_color_) {
                out << " fill=\""sv << *fill_color_
                    << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_
                    << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv
                    << *stroke_width_
                    << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv
                    << *stroke_linecap_
                    << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv
                    << *stroke_linejoin_
                    << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast ��������� ����������� *this � Owner&,
            // ���� ����� Owner � ��������� PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<dom::Color> fill_color_;
        std::optional<dom::Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    // --------------- ObjectContainer & Drawable ---------------

    class Object;

    // ����������� �����-��������� ��� ����������� ���������
    // ��������
    class ObjectContainer {
    public:
        // ������� �� ����� ���� ������������, �������
        // ������������ �����
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(obj));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        ~ObjectContainer() = default;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& oc) const = 0;

        virtual ~Drawable() = default;
    };

    // ------------------------- Object -------------------------

    // ����������� ������� ����� Object ������ ���
    // ���������������� �������� ���������� ����� SVG-���������
    // ��������� ������� "��������� �����" ��� ������ �����������
    // ����
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context)
            const = 0;
    };

    // ------------------------- Circle -------------------------

    // ����� Circle ���������� ������� <circle> ��� �����������
    // ����� https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/circle
    class Circle final : public Object,
        public PathProps<Circle> {
    public:
        Circle& SetCenter(dom::Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        dom::Point center_;
        double radius_ = 1.0;
    };

    // ------------------------ Polyline ------------------------

    // ����� Polyline ���������� ������� <polyline> ���
    // ����������� ������� �����
    // https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/polyline
    class Polyline final : public Object,
        public PathProps<Polyline> {
    public:
        Polyline& AddPoint(dom::Point point);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        std::vector<dom::Point> points_;
    };

    // -------------------------- Text --------------------------

    // ����� Text ���������� ������� <text> ��� �����������
    // ������ https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/text
    class Text final : public Object,
        public PathProps<Text> {
    public:
        Text() = default;

        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(dom::Point pos);

        // ����� �������� ������������ ������� �����
        // (�������� dx, dy)
        Text& SetOffset(dom::Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������
        // ������ ���� text)
        Text& SetData(std::string data);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        dom::Point pos_ = { 0.0, 0.0 };
        dom::Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_ = ""s;
        std::string font_weight_ = ""s;
        std::string data_ = ""s;
    };

    // ------------------------ Document ------------------------

    class Document : public ObjectContainer {
    public:
        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

} // namespace svg