#pragma once

#include <algorithm>

#include "svg.h"
#include "json.h"
#include "geo.h"
#include "domain.h"

namespace catalogue::renderer
{
    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector
    {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                        double max_height, double padding);

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer
    {
    public:
        struct RenderSettings
        {
            double width = 0;
            double height = 0;
            double padding = 0;
            double line_width = 0;
            double stop_radius = 0;
            int bus_label_font_size = 0;
            std::vector<double> bus_label_offset;
            int stop_label_font_size = 0;
            std::vector<double> stop_label_offset;
            svg::Color underlayer_color;
            double underlayer_width = 0;
            std::vector<svg::Color> color_palette;
        };

        MapRenderer(const json::Node &render_settings);

        MapRenderer(const RenderSettings &render_settings);

        double GetWidth() const;

        double GetHeight() const;

        double GetPadding() const;

        std::vector<svg::Polyline> DrawRoutes(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const domain::Bus *> &buses) const;

        std::vector<svg::Text> DrawNameBuses(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const domain::Bus *> &buses) const;

        std::vector<svg::Circle> DrawStopSymbols(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const geo::Coordinates *> &stops) const;

        std::vector<svg::Text> DrawNameStop(const renderer::SphereProjector &sphere_projector, const std::map<std::string_view, const geo::Coordinates *> &stops) const;

        const RenderSettings &GetRenderSettings() const;

    private:
        RenderSettings render_settings_;

        svg::Color ReadColor(json::Node color);

        RenderSettings SetRenderSettings(const json::Dict &render_settings);
    };

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                                     double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end)
        {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
                                                             { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
                                                             { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_))
        {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat))
        {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom)
        {
            zoom_coeff_ = *height_zoom;
        }
    }
}