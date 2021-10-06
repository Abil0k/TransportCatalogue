#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext
    {
        RenderContext(std::ostream &out)
            : out(out)
        {
        }

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent)
        {
        }

        RenderContext Indented() const
        {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t gr, uint8_t bl)
            : red(r), green(gr), blue(bl)
        {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba
    {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t gr, uint8_t bl, double op)
            : red(r), green(gr), blue(bl), opacity(op)
        {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{"none"};

    struct OstreamColorPrinter
    {
        std::ostream &out;

        void operator()(std::monostate) const;

        void operator()(std::string color) const;

        void operator()(Rgb color) const;

        void operator()(Rgba color) const;
    };

    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &stroke_line_join);

    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &stroke_line_cap);

    template <typename Owner>
    class PathProps
    {
    public:
        Owner &SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner &SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner &SetStrokeWidth(double width)
        {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner &SetStrokeLineCap(StrokeLineCap line_cap)
        {
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner &SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream &out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv;
                std::visit(OstreamColorPrinter{out}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv;
                std::visit(OstreamColorPrinter{out}, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_)
            {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_)
            {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner &AsOwner()
        {
            return static_cast<Owner &>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    class Object
    {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        Polyline &AddPoint(Point point);

    private:
        void RenderObject(const RenderContext &context) const override;
        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text>
    {
    public:
        Text &SetPosition(Point pos);

        Text &SetOffset(Point offset);

        Text &SetFontSize(uint32_t size);

        Text &SetFontFamily(std::string font_family);

        Text &SetFontWeight(std::string font_weight);

        Text &SetData(std::string data);

    private:
        void RenderObject(const RenderContext &context) const override;
        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_;
    };

    class ObjectContainer
    {
    public:
        template <typename Object>
        void Add(Object object);

        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

    protected:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    struct Drawable
    {
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer &container) const = 0;
    };

    struct Document : public ObjectContainer
    {
        void AddPtr(std::unique_ptr<Object> &&obj);

        void Render(std::ostream &out) const;
    };
}

template <typename Object>
void svg::ObjectContainer::Add(Object object)
{
    AddPtr(std::make_unique<Object>(std::move(object)));
}