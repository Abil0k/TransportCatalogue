#include <sstream>

#include "request_handler.h"
#include "svg.h"
#include "json_builder.h"

namespace handler
{
    using namespace std::string_literals;

    RequestHandler::RequestHandler(const catalogue::TransportCatalogue &transport_catalogue, const catalogue::renderer::MapRenderer &renderer,
                                   const catalogue::tr_router::TransoprtRouter &transport_router, const graph::Router<double> &router)
        : transport_catalogue_(transport_catalogue), renderer_(renderer), transport_router_(transport_router), router_(router)
    {
    }

    json::Node RequestHandler::CollectStopInformation(const domain::StopInformation &stop, int request_id)
    {
        if (!stop.name_stop.empty())
        {
            json::Array buses;
            buses.reserve(stop.buses.size());
            for (const auto bus : stop.buses)
            {
                buses.emplace_back(std::string{bus});
            }
            return json::Builder{}.StartDict().Key("buses"s).Value(buses).Key("request_id"s).Value(request_id).EndDict().Build();
        }
        else
        {
            return json::Builder{}.StartDict().Key("request_id"s).Value(request_id).Key("error_message"s).Value("not found"s).EndDict().Build();
        }
    }

    json::Node RequestHandler::CollectBusInformation(const domain::BusInformation &bus, int request_id)
    {
        if (!bus.name_bus.empty())
        {
            return json::Builder{}.StartDict().Key("curvature"s).Value(bus.curvature).Key("request_id"s).Value(request_id).Key("route_length"s).Value(bus.route_length).Key("stop_count"s).Value(bus.stops_on_route).Key("unique_stop_count"s).Value(bus.unique_stops).EndDict().Build();
        }
        else
        {
            return json::Builder{}.StartDict().Key("request_id"s).Value(request_id).Key("error_message"s).Value("not found"s).EndDict().Build();
        }
    }

    json::Node RequestHandler::CollectRouteInformation(const domain::RouteInformation &route, int request_id)
    {
        if (route.route_found)
        {
            json::Builder builder;
            builder.StartDict().Key("request_id"s).Value(request_id).Key("total_time"s).Value(route.total_time).Key("items"s).StartArray();
            if (!route.edges_info.empty())
            {
                builder.StartDict().Key("type"s).Value("Wait"s).Key("stop_name"s).Value(std::string{route.edges_info[0].stop_from}).Key("time"s).Value(route.bus_wait_time).EndDict();
                for (size_t i = 0; i + 1 < route.edges_info.size(); ++i)
                {
                    builder.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(std::string{route.edges_info[i].name_bus}).Key("span_count"s).Value(route.edges_info[i].span_count).Key("time").Value(route.edges_info[i].time).EndDict();
                    builder.StartDict().Key("type"s).Value("Wait"s).Key("stop_name"s).Value(std::string{route.edges_info[i].stop_to}).Key("time"s).Value(route.bus_wait_time).EndDict();
                }
                builder.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(std::string{route.edges_info.back().name_bus}).Key("span_count"s).Value(route.edges_info.back().span_count).Key("time").Value(route.edges_info.back().time).EndDict();
            }
            return builder.EndArray().EndDict().Build();
        }
        else
        {
            return json::Builder{}.StartDict().Key("request_id"s).Value(request_id).Key("error_message"s).Value("not found"s).EndDict().Build();
        }
    }

    svg::Document RequestHandler::RenderMap() const
    {
        svg::Document result;
        std::map<std::string_view, const geo::Coordinates *> stops = transport_catalogue_.FindAllWorkingStops();
        std::map<std::string_view, const domain::Bus *> buses = transport_catalogue_.FindAllWorkingBuses();

        std::vector<geo::Coordinates> coordinates;
        for (const auto &stop : stops)
        {
            coordinates.emplace_back(*stop.second);
        }

        catalogue::renderer::SphereProjector sphere_projector(coordinates.begin(), coordinates.end(), renderer_.GetWidth(), renderer_.GetHeight(), renderer_.GetPadding());
        for (auto rout : renderer_.DrawRoutes(sphere_projector, buses))
        {
            result.Add(rout);
        }
        for (auto name_bus : renderer_.DrawNameBuses(sphere_projector, buses))
        {
            result.Add(name_bus);
        }
        for (auto stop_symbol : renderer_.DrawStopSymbols(sphere_projector, stops))
        {
            result.Add(stop_symbol);
        }
        for (auto name_stop : renderer_.DrawNameStop(sphere_projector, stops))
        {
            result.Add(name_stop);
        }
        return result;
    }

    json::Array RequestHandler::FindInformation(const json::Node &json_data_base)
    {
        auto stat_requests = json_data_base.AsArray();
        json::Array information_found;
        information_found.reserve(stat_requests.size());
        for (const auto &request : stat_requests)
        {
            if (request.AsDict().at("type"s).AsString() == "Stop"s)
            {
                auto stop = transport_catalogue_.FindStopInformation(request.AsDict().at("name"s).AsString());
                information_found.emplace_back(CollectStopInformation(stop, request.AsDict().at("id"s).AsInt()));
            }
            else if (request.AsDict().at("type"s).AsString() == "Bus"s)
            {
                auto bus = transport_catalogue_.FindBusInformation(request.AsDict().at("name"s).AsString());
                information_found.emplace_back(CollectBusInformation(bus, request.AsDict().at("id"s).AsInt()));
            }
            else if (request.AsDict().at("type"s).AsString() == "Map"s)
            {

                svg::Document map = RenderMap();
                std::ostringstream strm;
                map.Render(strm);
                information_found.emplace_back(json::Builder{}.StartDict().Key("map"s).Value(strm.str()).Key("request_id"s).Value(request.AsDict().at("id"s).AsInt()).EndDict().Build());
            }
            else if (request.AsDict().at("type"s).AsString() == "Route"s)
            {
                std::optional<graph::Router<double>::RouteInfo> route;
                if (request.AsDict().at("from"s).AsString() == request.AsDict().at("to"s).AsString())
                {
                    route = {0, {}};
                }
                else if (transport_router_.StopIsWorking(request.AsDict().at("from"s).AsString()) && transport_router_.StopIsWorking(request.AsDict().at("to"s).AsString()))
                {
                    route = router_.BuildRoute(transport_router_.GetStopId(request.AsDict().at("from"s).AsString()), transport_router_.GetStopId(request.AsDict().at("to"s).AsString()));
                }
                information_found.emplace_back(CollectRouteInformation(transport_router_.FindRouteInformation(route), request.AsDict().at("id"s).AsInt()));
            }
        }
        return information_found;
    }
}