#include <transport_router.pb.h>

#include "serialization.h"

namespace serialization
{
    using namespace std::string_literals;
    proto_catalogue::TransportCatalogue CreateProtoCatalogue(const catalogue::TransportCatalogue &transport_catalogue)
    {
        const std::unordered_map<std::string, geo::Coordinates> &stops = transport_catalogue.GetAllStops();
        const std::unordered_map<std::string, domain::Bus> &buses = transport_catalogue.GetAllBuses();
        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> &distance_between_stops = transport_catalogue.GetDistanceBetweenStops();
        std::unordered_map<std::string_view, int> stops_id;
        for (const auto &[name_stop, coordinates] : stops)
        {
            stops_id[name_stop] = stops_id.size();
        }

        proto_catalogue::TransportCatalogue proto_catalogue;
        for (const auto &[name_stop, coordinates] : stops)
        {
            proto_catalogue::Stop stop;
            stop.set_name(name_stop);
            stop.set_id(stops_id.at(name_stop));
            proto_catalogue::Coordinates stop_coordinates;
            stop_coordinates.set_lat(coordinates.lat);
            stop_coordinates.set_lng(coordinates.lng);
            *stop.mutable_coordinates() = std::move(stop_coordinates);
            if (distance_between_stops.count(name_stop))
            {
                for (const auto &[name, dist] : distance_between_stops.at(name_stop))
                {
                    proto_catalogue::DistanceToStops distance_to_stops;
                    distance_to_stops.set_stop_to_id(stops_id.at(name));
                    distance_to_stops.set_distance(dist);
                    stop.add_distance_to_stops()->CopyFrom(distance_to_stops);
                }
            }
            proto_catalogue.add_stops()->CopyFrom(stop);
        }

        for (const auto &[name_bus, bus_info] : buses)
        {
            proto_catalogue::Bus bus;
            bus.set_name(std::string{name_bus});
            bus.set_is_circular(bus_info.is_circular);
            for (const auto &stop : bus_info.stops)
            {
                bus.add_stops(stops_id.at(stop.first));
            }
            proto_catalogue.add_buses()->CopyFrom(bus);
        }
        return proto_catalogue;
    }

    proto_svg::Color GetProtoColor(const svg::Color &color)
    {
        proto_svg::Color proto_color;
        if (std::holds_alternative<std::string>(color))
        {
            proto_color.set_type("string"s);
            proto_color.set_color(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color))
        {
            proto_color.set_type("rgb"s);
            svg::Rgb clr = std::get<svg::Rgb>(color);
            proto_color.set_red(clr.red);
            proto_color.set_green(clr.green);
            proto_color.set_blue(clr.blue);
        }
        else if (std::holds_alternative<svg::Rgba>(color))
        {
            proto_color.set_type("rgba"s);
            svg::Rgba clr = std::get<svg::Rgba>(color);
            proto_color.set_red(clr.red);
            proto_color.set_green(clr.green);
            proto_color.set_blue(clr.blue);
            proto_color.set_opacity(clr.opacity);
        }
        return proto_color;
    }

    proto_map_renderer::RenderSettings CreateProtoRenderSettings(const catalogue::renderer::MapRenderer::RenderSettings &render_settings)
    {
        proto_map_renderer::RenderSettings proto_render_settings;
        proto_render_settings.set_width(render_settings.width);
        proto_render_settings.set_height(render_settings.height);
        proto_render_settings.set_padding(render_settings.padding);
        proto_render_settings.set_line_width(render_settings.line_width);
        proto_render_settings.set_stop_radius(render_settings.stop_radius);
        proto_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
        for (const auto offset : render_settings.bus_label_offset)
        {
            proto_render_settings.add_bus_label_offset(offset);
        }
        proto_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
        for (const auto offset : render_settings.stop_label_offset)
        {
            proto_render_settings.add_stop_label_offset(offset);
        }
        *proto_render_settings.mutable_underlayer_color() = GetProtoColor(render_settings.underlayer_color);
        proto_render_settings.set_underlayer_width(render_settings.underlayer_width);
        for (const auto &color : render_settings.color_palette)
        {
            proto_render_settings.add_color_palette()->CopyFrom(GetProtoColor(color));
        }
        return proto_render_settings;
    }

    proto_tr_router::TransportRouter CreateProtoTransportRouter(const catalogue::tr_router::TransoprtRouter &transport_router)
    {
        proto_tr_router::TransportRouter proto_router;
        std::unordered_map<std::string_view, size_t> buses_id;
        std::unordered_map<std::string_view, size_t> stops_id;

        proto_router.set_bus_wait_time(transport_router.GetBusWaitTime());
        const std::unordered_map<size_t, domain::EdgeInfo> &edges_info = transport_router.GetEdgesInfo();
        for (const auto &[_, edge_info] : edges_info)
        {
            proto_tr_router::EdgeInfo proto_edge_info;
            if (!buses_id.count(edge_info.name_bus))
            {
                proto_tr_router::Bus proto_bus;
                proto_bus.set_id(buses_id.size());
                proto_bus.set_name(std::string{edge_info.name_bus});
                proto_router.add_buses()->CopyFrom(proto_bus);
                buses_id[edge_info.name_bus] = buses_id.size();
            }
            proto_edge_info.set_bus(buses_id.at(edge_info.name_bus));
            proto_edge_info.set_span_count(edge_info.span_count);
            proto_edge_info.set_time(edge_info.time);
            if (!stops_id.count(edge_info.stop_from))
            {
                proto_tr_router::Stop proto_stop;
                proto_stop.set_id(stops_id.size());
                proto_stop.set_name(std::string{edge_info.stop_from});
                proto_router.add_stops()->CopyFrom(proto_stop);
                stops_id[edge_info.stop_from] = stops_id.size();
            }
            proto_edge_info.set_stop_from(stops_id.at(edge_info.stop_from));
            if (!stops_id.count(edge_info.stop_to))
            {
                proto_tr_router::Stop proto_stop;
                proto_stop.set_id(stops_id.size());
                proto_stop.set_name(std::string{edge_info.stop_to});
                proto_router.add_stops()->CopyFrom(proto_stop);
                stops_id[edge_info.stop_to] = stops_id.size();
            }
            proto_edge_info.set_stop_to(stops_id.at(edge_info.stop_to));
            proto_router.add_edges_info()->CopyFrom(proto_edge_info);
        }
        return proto_router;
    }

    catalogue::TransportCatalogue DeserializeCatalogue(const proto_catalogue::TransportCatalogue &proto_catalogue)
    {
        catalogue::TransportCatalogue transport_catalogue;
        std::unordered_map<int, std::string> id_stops;
        for (const auto &stop : proto_catalogue.stops())
        {
            domain::StopInputInfo stop_info;
            stop_info.name_stop = stop.name();
            geo::Coordinates coordinates;
            coordinates.lat = stop.coordinates().lat();
            coordinates.lng = stop.coordinates().lng();
            stop_info.coordinates = coordinates;
            transport_catalogue.AddStop(stop_info);
            id_stops[stop.id()] = stop.name();
        }
        for (const auto &stop : proto_catalogue.stops())
        {
            domain::StopInputInfo stop_info;
            stop_info.name_stop = stop.name();
            for (const auto &dist : stop.distance_to_stops())
            {
                stop_info.distance_to_other_stops[id_stops.at(dist.stop_to_id())] = dist.distance();
            }
            transport_catalogue.AddDistanceBetweenStop(stop_info);
        }
        for (const auto &bus : proto_catalogue.buses())
        {
            domain::BusInputInfo bus_info;
            bus_info.name_bus = bus.name();
            bus_info.is_circular = bus.is_circular();
            for (const auto &stop : bus.stops())
            {
                bus_info.stops.push_back(id_stops.at(stop));
            }
            transport_catalogue.AddBus(bus_info);
        }
        return transport_catalogue;
    }

    svg::Color GetColor(proto_svg::Color proto_color)
    {
        svg::Color color;
        if (proto_color.type() == "string"s)
        {
            color = proto_color.color();
        }
        else if (proto_color.type() == "rgb"s)
        {
            svg::Rgb clr;
            clr.red = proto_color.red();
            clr.green = proto_color.green();
            clr.blue = proto_color.blue();
            color = clr;
        }
        else
        {
            svg::Rgba clr;
            clr.red = proto_color.red();
            clr.green = proto_color.green();
            clr.blue = proto_color.blue();
            clr.opacity = proto_color.opacity();
            color = clr;
        }
        return color;
    }

    catalogue::renderer::MapRenderer::RenderSettings DeserializeMapRenderer(const proto_map_renderer::RenderSettings &proto_render_settings)
    {
        catalogue::renderer::MapRenderer::RenderSettings render_settings;
        render_settings.width = proto_render_settings.width();
        render_settings.height = proto_render_settings.height();
        render_settings.padding = proto_render_settings.padding();
        render_settings.line_width = proto_render_settings.line_width();
        render_settings.stop_radius = proto_render_settings.stop_radius();
        render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
        for (const auto offset : proto_render_settings.bus_label_offset())
        {
            render_settings.bus_label_offset.push_back(offset);
        }
        render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();
        for (const auto offset : proto_render_settings.stop_label_offset())
        {
            render_settings.stop_label_offset.push_back(offset);
        }
        render_settings.underlayer_color = GetColor(proto_render_settings.underlayer_color());
        render_settings.underlayer_width = proto_render_settings.underlayer_width();
        for (const auto color : proto_render_settings.color_palette())
        {
            render_settings.color_palette.push_back(GetColor(color));
        }
        return render_settings;
    }

    using Graph = graph::DirectedWeightedGraph<double>;

    Graph DeserializeTransportRouter(const proto_tr_router::TransportRouter &proto_router, catalogue::tr_router::TransoprtRouter &tr_router)
    {
        std::unordered_map<size_t, std::string_view> buses_id;
        std::unordered_map<size_t, std::string_view> stops_id;
        for (const auto &stop : proto_router.stops())
        {
            stops_id[stop.id()] = stop.name();
        }
        for (const auto &bus : proto_router.buses())
        {
            buses_id[bus.id()] = bus.name();
        }
        double bus_wait_time = proto_router.bus_wait_time();
        tr_router.SetBusWaitTime(bus_wait_time);
        const std::unordered_map<std::string, geo::Coordinates> &stops = tr_router.GetTransoprtCatalogue().GetAllStops();
        const std::unordered_map<std::string, domain::Bus> &buses = tr_router.GetTransoprtCatalogue().GetAllBuses();
        Graph graph(stops_id.size());
        for (const auto &proto_edge_info : proto_router.edges_info())
        {
            domain::EdgeInfo edge_info;
            edge_info.span_count = proto_edge_info.span_count();
            edge_info.time = proto_edge_info.time();
            edge_info.name_bus = buses.find(std::string{buses_id.at(proto_edge_info.bus())})->first;
            edge_info.stop_from = stops.find(std::string{stops_id.at(proto_edge_info.stop_from())})->first;
            edge_info.stop_to = stops.find(std::string{stops_id.at(proto_edge_info.stop_to())})->first;
            graph::Edge<double> edge;
            edge.from = tr_router.GetStopId(edge_info.stop_from);
            edge.to = tr_router.GetStopId(edge_info.stop_to);
            edge.weight = edge_info.time + bus_wait_time;
            tr_router.AddEdgeInfo(graph.AddEdge(edge), edge_info);
        }
        return graph;
    }
}