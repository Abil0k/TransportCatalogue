#pragma once

#include "transport_router.h"
#include "map_renderer.h"

namespace handler
{
    class RequestHandler
    {
    public:
        RequestHandler(const catalogue::TransportCatalogue &transport_catalogue, const catalogue::renderer::MapRenderer &renderer,
                       const catalogue::tr_router::TransoprtRouter &transport_router, const graph::Router<double> &router);

        svg::Document RenderMap() const;

        json::Array FindInformation(const json::Node &stat_requests);

    private:
        const catalogue::TransportCatalogue &transport_catalogue_;
        const catalogue::renderer::MapRenderer &renderer_;
        const catalogue::tr_router::TransoprtRouter &transport_router_;
        const graph::Router<double> &router_;

        json::Node CollectStopInformation(const domain::StopInformation &stop, int request_id);

        json::Node CollectBusInformation(const domain::BusInformation &bus, int request_id);

        json::Node CollectRouteInformation(const domain::RouteInformation &route, int request_id);
    };
}