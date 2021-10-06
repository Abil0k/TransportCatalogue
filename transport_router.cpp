#include "transport_router.h"

using namespace std::string_literals;

namespace catalogue::tr_router
{
    using Graph = graph::DirectedWeightedGraph<double>;

    TransoprtRouter::TransoprtRouter(const catalogue::TransportCatalogue &transport_catalogue, const json::Node &routing_settings)
        : transport_catalogue_(transport_catalogue),
          bus_velocity_(routing_settings.AsDict().at("bus_velocity"s).AsDouble()),
          bus_wait_time_(routing_settings.AsDict().at("bus_wait_time"s).AsDouble())
    {
        SetStopsId();
    }

    TransoprtRouter::TransoprtRouter(const catalogue::TransportCatalogue &transport_catalogue)
        : transport_catalogue_(transport_catalogue)
    {
        SetStopsId();
    }

    Graph TransoprtRouter::CreateGraph()
    {
        Graph graph(stops_id_.size());
        std::map<std::string_view, const domain::Bus *> buses = transport_catalogue_.FindAllWorkingBuses();
        for (const auto &bus : buses)
        {
            for (size_t i = 0; i + 1 < bus.second->stops.size(); ++i)
            {
                double weight = bus_wait_time_;
                for (size_t j = i + 1; j < bus.second->stops.size(); ++j)
                {
                    if (bus.second->stops[i].first == bus.second->stops[j].first)
                    {
                        double weight = bus_wait_time_;
                        for (size_t k = j + 1; k < bus.second->stops.size(); ++k)
                        {
                            graph::Edge<double> edge = CreateEdge(weight, bus, i, k, true);
                            domain::EdgeInfo edge_info = CountEdgeInfo(weight, bus, i, k);
                            edge_info.span_count = k - j;
                            edges_info_[graph.AddEdge(edge)] = edge_info;
                        }
                    }
                    graph::Edge<double> edge = CreateEdge(weight, bus, i, j, true);
                    domain::EdgeInfo edge_info = CountEdgeInfo(weight, bus, i, j);
                    edge_info.span_count = j - i;
                    edges_info_[graph.AddEdge(edge)] = edge_info;
                }
            }
            if (!bus.second->is_circular)
            {
                for (size_t i = bus.second->stops.size() - 1; i > 0; --i)
                {
                    double weight = bus_wait_time_;
                    for (size_t j = i - 1; j + 1 > 0; --j)
                    {
                        if (bus.second->stops[i].first == bus.second->stops[j].first)
                        {
                            double weight = bus_wait_time_;
                            for (size_t k = j - 1; k + 1 > 0; --k)
                            {
                                graph::Edge<double> edge = CreateEdge(weight, bus, i, k, false);
                                domain::EdgeInfo edge_info = CountEdgeInfo(weight, bus, i, k);
                                edge_info.span_count = j - k;
                                edges_info_[graph.AddEdge(edge)] = edge_info;
                            }
                        }
                        graph::Edge<double> edge = CreateEdge(weight, bus, i, j, false);
                        domain::EdgeInfo edge_info = CountEdgeInfo(weight, bus, i, j);
                        edge_info.span_count = i - j;
                        edges_info_[graph.AddEdge(edge)] = edge_info;
                    }
                }
            }
        }
        return graph;
    }

    bool TransoprtRouter::StopIsWorking(std::string_view name_stop) const
    {
        return stops_id_.count(name_stop);
    }

    size_t TransoprtRouter::GetStopId(std::string_view name_stop) const
    {
        return stops_id_.at(name_stop);
    }

    domain::EdgeInfo TransoprtRouter::GetEdgeInfo(double edge) const
    {
        return edges_info_.at(edge);
    }

    domain::RouteInformation TransoprtRouter::FindRouteInformation(const std::optional<graph::Router<double>::RouteInfo> &route) const
    {
        domain::RouteInformation route_info;
        if (route.has_value())
        {
            route_info.total_time = route->weight;
            route_info.bus_wait_time = bus_wait_time_;
            for (const auto &edge : route->edges)
            {
                route_info.edges_info.emplace_back(GetEdgeInfo(edge));
            }
            route_info.route_found = true;
        }
        return route_info;
    }

    const std::unordered_map<size_t, domain::EdgeInfo> &TransoprtRouter::GetEdgesInfo() const
    {
        return edges_info_;
    }

    double TransoprtRouter::GetBusWaitTime() const
    {
        return bus_wait_time_;
    }

    void TransoprtRouter::AddEdgeInfo(size_t id, const domain::EdgeInfo &edge_info)
    {
        edges_info_[id] = edge_info;
    }

    void TransoprtRouter::SetBusWaitTime(double bus_wait_time)
    {
        bus_wait_time_ = bus_wait_time;
    }

    void TransoprtRouter::SetBusVelocity(double bus_velocity)
    {
        bus_velocity_ = bus_velocity;
    }

    const catalogue::TransportCatalogue &TransoprtRouter::GetTransoprtCatalogue() const
    {
        return transport_catalogue_;
    }

    void TransoprtRouter::SetStopsId()
    {
        std::map<std::string_view, const geo::Coordinates *> all_working_stops = transport_catalogue_.FindAllWorkingStops();
        for (const auto &[stop, _] : all_working_stops)
        {
            stops_id_[stop] = stops_id_.size();
        }
    }

    graph::Edge<double> TransoprtRouter::CreateEdge(double &weight, const std::pair<const std::string_view, const domain::Bus *> &bus, size_t from, size_t to, bool it_straight)
    {
        graph::Edge<double> edge;
        edge.from = stops_id_.at(bus.second->stops[from].first);
        edge.to = stops_id_.at(bus.second->stops[to].first);
        if (it_straight)
        {
            weight += (transport_catalogue_.CalculateDistance(bus.second->stops[to - 1], bus.second->stops[to]) * 1.0) /
                      (bus_velocity_ / 0.06);
        }
        else
        {
            weight += (transport_catalogue_.CalculateDistance(bus.second->stops[to + 1], bus.second->stops[to]) * 1.0) /
                      (bus_velocity_ / 0.06);
        }
        edge.weight = weight;
        return edge;
    }

    domain::EdgeInfo TransoprtRouter::CountEdgeInfo(double weight, const std::pair<const std::string_view, const domain::Bus *> &bus, size_t from, size_t to)
    {
        domain::EdgeInfo edge_info;
        edge_info.name_bus = bus.first;
        edge_info.time = weight - bus_wait_time_;
        edge_info.stop_from = bus.second->stops[from].first;
        edge_info.stop_to = bus.second->stops[to].first;
        return edge_info;
    }
}