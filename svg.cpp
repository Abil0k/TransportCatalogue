#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void OstreamColorPrinter::operator()(std::monostate) const
    {
        out << "none"sv;
    }
    void OstreamColorPrinter::operator()(std::string color) const
    {
        out << color;
    }
    void OstreamColorPrinter::operator()(Rgb color) const
    {
        out << "rgb("sv << unsigned(color.red) << ","sv << unsigned(color.green) << ","sv << unsigned(color.blue) << ")"sv;
    }
    void OstreamColorPrinter::operator()(Rgba color) const
    {
        out << "rgba("sv << unsigned(color.red) << ","sv << unsigned(color.green) << ","sv << unsigned(color.blue) << ","sv << color.opacity << ")"sv;
    }

    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &stroke_line_join)
    {
        if (stroke_line_join == StrokeLineJoin::ARCS)
        {
            out << "arcs"sv;
        }
        else if (stroke_line_join == StrokeLineJoin::BEVEL)
        {
            out << "bevel"sv;
        }
        else if (stroke_line_join == StrokeLineJoin::MITER)
        {
            out << "miter"sv;
        }
        else if (stroke_line_join == StrokeLineJoin::MITER_CLIP)
        {
            out << "miter-clip"sv;
        }
        else if (stroke_line_join == StrokeLineJoin::ROUND)
        {
            out << "round"sv;
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &stroke_line_cap)
    {
        if (stroke_line_cap == StrokeLineCap::BUTT)
        {
            out << "butt"sv;
        }
        else if (stroke_line_cap == StrokeLineCap::ROUND)
        {
            out << "round"sv;
        }
        else if (stroke_line_cap == StrokeLineCap::SQUARE)
        {
            out << "square"sv;
        }
        return out;
    }

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline &Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty())
        {
            for (size_t i = 0; i + 1 < points_.size(); ++i)
            {
                out << points_[i].x << ","sv << points_[i].y << " "sv;
            }
            out << points_.back().x << ","sv << points_.back().y;
        }
        else
        {
            out << ""sv;
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text &Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data)
    {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;
        if (font_family_)
        {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if (font_weight_)
        {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << ">"sv << data_;
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object> &&obj)
    {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (const auto &obj : objects_)
        {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}