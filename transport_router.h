#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include "json.h"

namespace catalogue::tr_router
{
    class TransoprtRouter
    {
    private:
        using Graph = graph::DirectedWeightedGraph<double>;

    public:
        TransoprtRouter(const catalogue::TransportCatalogue &transport_catalogue, const json::Node &routing_settings);

        TransoprtRouter(const catalogue::TransportCatalogue &transport_catalogue);

        Graph CreateGraph();

        bool StopIsWorking(std::string_view name_stop) const;

        size_t GetStopId(std::string_view name_stop) const;

        domain::EdgeInfo GetEdgeInfo(double edge) const;

        domain::RouteInformation FindRouteInformation(const std::optional<graph::Router<double>::RouteInfo> &route) const;

        const std::unordered_map<size_t, domain::EdgeInfo> &GetEdgesInfo() const;

        double GetBusWaitTime() const;

        void AddEdgeInfo(size_t id, const domain::EdgeInfo &edge_info);

        void SetBusWaitTime(double bus_wait_time);

        void SetBusVelocity(double bus_velocity);

        const catalogue::TransportCatalogue &GetTransoprtCatalogue() const;

    private:
        const catalogue::TransportCatalogue &transport_catalogue_;
        double bus_velocity_ = 0;
        double bus_wait_time_ = 0;
        std::unordered_map<std::string_view, size_t> stops_id_;
        std::unordered_map<size_t, domain::EdgeInfo> edges_info_;

        void SetStopsId();

        graph::Edge<double> CreateEdge(double &weight, const std::pair<const std::string_view, const domain::Bus *> &bus,
                                       size_t from, size_t to, bool it_straight);

        domain::EdgeInfo CountEdgeInfo(double weight, const std::pair<const std::string_view, const domain::Bus *> &bus,
                                       size_t from, size_t to);
    };
}