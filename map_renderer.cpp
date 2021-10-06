#include "map_renderer.h"

#include "transport_catalogue.h"

namespace catalogue::renderer
{
    using namespace std::string_literals;

    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    MapRenderer::MapRenderer(const json::Node &render_settings)
        : render_settings_(SetRenderSettings(render_settings.AsDict()))
    {
    }

    MapRenderer::MapRenderer(const RenderSettings &render_settings)
        : render_settings_(render_settings)
    {
    }

    double MapRenderer::GetWidth() const
    {
        return render_settings_.width;
    }

    double MapRenderer::GetHeight() const
    {
        return render_settings_.height;
    }

    double MapRenderer::GetPadding() const
    {
        return render_settings_.padding;
    }

    std::vector<svg::Polyline> MapRenderer::DrawRoutes(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const domain::Bus *> &buses) const
    {
        std::vector<svg::Polyline> result;
        size_t color_num = 0;
        for (const auto &bus : buses)
        {
            svg::Polyline line;
            for (const auto &stop : bus.second->stops)
            {
                line.AddPoint(sphere_projector(*stop.second));
            }
            if (!bus.second->is_circular)
            {
                for (size_t i = bus.second->stops.size() - 1; i > 0; --i)
                {
                    line.AddPoint(sphere_projector(*bus.second->stops[i - 1].second));
                }
            }
            line.SetFillColor("none"s);
            line.SetStrokeColor(render_settings_.color_palette[color_num]);
            line.SetStrokeWidth(render_settings_.line_width);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            if (color_num + 1 < (render_settings_.color_palette.size()))
            {
                ++color_num;
            }
            else
            {
                color_num = 0;
            }

            result.emplace_back(line);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::DrawNameBuses(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const domain::Bus *> &buses) const
    {
        std::vector<svg::Text> result;
        size_t color_num = 0;
        for (const auto &bus : buses)
        {
            if (!bus.second->stops.empty())
            {
                svg::Text text;
                text.SetPosition(sphere_projector(*bus.second->stops[0].second));
                text.SetOffset(svg::Point{render_settings_.bus_label_offset[0], render_settings_.bus_label_offset[1]});
                text.SetFontSize(render_settings_.bus_label_font_size);
                text.SetFontFamily("Verdana"s);
                text.SetFontWeight("bold"s);
                text.SetData(std::string{bus.first});
                svg::Text text_underlayer = text;
                text_underlayer.SetFillColor(render_settings_.underlayer_color);
                text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
                text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
                text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                text.SetFillColor(render_settings_.color_palette[color_num]);
                if (color_num + 1 < render_settings_.color_palette.size())
                {
                    ++color_num;
                }
                else
                {
                    color_num = 0;
                }
                result.emplace_back(text_underlayer);
                result.emplace_back(text);
                if (bus.second->stops[0] != bus.second->stops.back())
                {
                    svg::Text text2 = text;
                    svg::Text text2_underlayer = text_underlayer;
                    text2.SetPosition(sphere_projector(*bus.second->stops.back().second));
                    text2_underlayer.SetPosition(sphere_projector(*bus.second->stops.back().second));
                    result.emplace_back(text2_underlayer);
                    result.emplace_back(text2);
                }
            }
        }
        return result;
    }

    std::vector<svg::Circle> MapRenderer::DrawStopSymbols(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const geo::Coordinates *> &stops) const
    {
        std::vector<svg::Circle> result;
        for (const auto &stop : stops)
        {
            svg::Circle circle;
            circle.SetCenter(sphere_projector(*stop.second));
            circle.SetRadius(render_settings_.stop_radius);
            circle.SetFillColor("white"s);
            result.emplace_back(circle);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::DrawNameStop(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const geo::Coordinates *> &stops) const
    {
        std::vector<svg::Text> result;
        for (const auto &stop : stops)
        {
            svg::Text text;
            text.SetPosition(sphere_projector(*stop.second));
            text.SetOffset(svg::Point{render_settings_.stop_label_offset[0], render_settings_.stop_label_offset[1]});
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana"s);
            text.SetData(std::string{stop.first});
            svg::Text text_underlayer = text;
            text_underlayer.SetFillColor(render_settings_.underlayer_color);
            text_underlayer.SetStrokeColor(render_settings_.underlayer_color);
            text_underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            text_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor("black"s);
            result.emplace_back(text_underlayer);
            result.emplace_back(text);
        }
        return result;
    }

    const MapRenderer::RenderSettings &MapRenderer::GetRenderSettings() const
    {
        return render_settings_;
    }

    svg::Color MapRenderer::ReadColor(json::Node color)
    {
        svg::Color result;

        if (color.IsString())
        {
            result = color.AsString();
        }
        else
        {
            const auto &color_link = color.AsArray();
            uint8_t red = static_cast<uint8_t>(color_link[0].AsInt());
            uint8_t green = static_cast<uint8_t>(color_link[1].AsInt());
            uint8_t blue = static_cast<uint8_t>(color_link[2].AsInt());
            if (color_link.size() == 3)
            {
                result = svg::Rgb{red, green, blue};
            }
            else
            {
                double opacity = color_link[3].AsDouble();
                result = svg::Rgba{red, green, blue, opacity};
            }
        }
        return result;
    }

    MapRenderer::RenderSettings MapRenderer::SetRenderSettings(const json::Dict &render_settings)
    {
        RenderSettings result;
        result.width = render_settings.at("width"s).AsDouble();
        result.height = render_settings.at("height"s).AsDouble();
        result.padding = render_settings.at("padding"s).AsDouble();
        result.line_width = render_settings.at("line_width"s).AsDouble();
        result.stop_radius = render_settings.at("stop_radius"s).AsDouble();
        result.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
        const auto &bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
        result.bus_label_offset.emplace_back(bus_label_offset[0].AsDouble());
        result.bus_label_offset.emplace_back(bus_label_offset[1].AsDouble());
        result.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
        const auto &stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
        result.stop_label_offset.emplace_back(stop_label_offset[0].AsDouble());
        result.stop_label_offset.emplace_back(stop_label_offset[1].AsDouble());
        result.underlayer_color = ReadColor(render_settings.at("underlayer_color"s));
        result.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
        for (const auto &color : render_settings.at("color_palette"s).AsArray())
        {
            result.color_palette.emplace_back(ReadColor(color));
        }
        return result;
    }
}