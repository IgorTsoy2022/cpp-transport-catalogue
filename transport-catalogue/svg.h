#pragma once

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

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y) {
        }
        double x = 0;
        double y = 0;
    };

    // Вспомогательная структура, хранящая контекст для вывода
    // SVG-документа с отступами.
    // Хранит ссылку на поток вывода, текущее значение и шаг
    // отступа при выводе элемента 
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

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r), green(g), blue(b)
        {};

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b,
            double o)
            : red(r), green(g), blue(b), opacity(o)
        {};

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string,
        svg::Rgb, svg::Rgba>;

    struct ColorOutputStream {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none"sv;
        }

        void operator()(const std::string value) const {
            out << value;
        }

        void operator()(const svg::Rgb& value) const {
            out << "rgb("sv << +value.red << ","sv
                << +value.green << ","sv
                << +value.blue << ")"sv;
        }

        void operator()(const svg::Rgba& value) const {
            out << "rgba("sv << +value.red << ","sv
                << +value.green << ","sv
                << +value.blue << ","sv
                << +value.opacity << ")"sv;
        }
    };

    std::ostream& operator<<(std::ostream& out,
        const svg::Color& value);

    // Объявив в заголовочном файле константу со спецификатором
    // inline, мы сделаем так, что она будет одной на все единицы
    // трансляции, которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет
    // использовать свою копию этой константы
    inline const Color NoneColor = std::monostate{};

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
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
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
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    // --------------- ObjectContainer & Drawable ---------------

    class Object;

    // Абстрактный класс-интерфейс для подключения различных
    // объектов
    class ObjectContainer {
    public:
        // шаблоны не могут быть виртуальными, поэтому
        // определяется здесь
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

    // Абстрактный базовый класс Object служит для
    // унифицированного хранения конкретных тегов SVG-документа
    // Реализует паттерн "Шаблонный метод" для вывода содержимого
    // тега
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context)
            const = 0;
    };

    // ------------------------- Circle -------------------------

    // Класс Circle моделирует элемент <circle> для отображения
    // круга https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/circle
    class Circle final : public Object,
        public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    // ------------------------ Polyline ------------------------

    // Класс Polyline моделирует элемент <polyline> для
    // отображения ломаных линий
    // https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/polyline
    class Polyline final : public Object,
        public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    // -------------------------- Text --------------------------

    // Класс Text моделирует элемент <text> для отображения
    // текста https://developer.mozilla.org/en-US/docs/Web/SVG/
    // Element/text
    class Text final : public Object,
        public PathProps<Text> {
    public:
        Text() = default;

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки
        // (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается
        // внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(
            const RenderContext& context) const override;

        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_ = ""s;
        std::string font_weight_ = ""s;
        std::string data_ = ""s;
    };

    // ------------------------ Document ------------------------

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg