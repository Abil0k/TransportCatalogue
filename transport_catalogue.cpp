#include <algorithm>

#include "transport_catalogue.h"

namespace catalogue
{
    using namespace domain;
    void TransportCatalogue::AddStop(StopInputInfo stop_info)
    {
        stops_[std::move(stop_info.name_stop)] = std::move(stop_info.coordinates);
    }

    void TransportCatalogue::AddDistanceBetweenStop(const StopInputInfo &stop_info)
    {
        if (!stop_info.distance_to_other_stops.empty())
        {
            for (const auto &[other_stop, distance] : stop_info.distance_to_other_stops)
            {
                distance_between_stops_[stops_.find(stop_info.name_stop)->first][stops_.find(other_stop)->first] = distance;
            }
        }
    }

    void TransportCatalogue::AddBus(const BusInputInfo &bus_info)
    {
        Bus bus;
        bus.is_circular = bus_info.is_circular;
        bus.stops.reserve(bus_info.stops.size());
        for (const std::string &stop : bus_info.stops)
        {
            bus.stops.push_back({stops_.find(stop)->first, &stops_.find(stop)->second});
        }
        buses_[bus_info.name_bus] = std::move(bus);
        for (const std::string &stop : bus_info.stops)
        {
            buses_passing_stops_[stops_.find(stop)->first].insert(buses_.find(bus_info.name_bus)->first);
        }
    }

    BusInformation TransportCatalogue::FindBusInformation(const std::string &query) const
    {
        BusInformation bus_information;
        Bus bus;
        if (buses_.count(query))
        {
            bus = buses_.at(query);
            bus_information.name_bus = buses_.find(query)->first;
            std::tuple<int, int, double, double> info = CalculateBusInformation(bus);
            bus_information.stops_on_route = std::get<0>(info);
            bus_information.unique_stops = std::get<1>(info);
            bus_information.route_length = std::get<2>(info);
            bus_information.curvature = std::get<3>(info);
        }
        return bus_information;
    }

    StopInformation TransportCatalogue::FindStopInformation(const std::string &query) const
    {
        StopInformation stop_information;
        if (stops_.count(query))
        {
            if (buses_passing_stops_.count(query))
            {
                for (auto bus : buses_passing_stops_.at(query))
                {
                    stop_information.buses.insert(bus);
                }
            }
            stop_information.name_stop = stops_.find(query)->first;
        }
        return stop_information;
    }

    std::map<std::string_view, const geo::Coordinates *> TransportCatalogue::FindAllWorkingStops() const
    {
        std::map<std::string_view, const geo::Coordinates *> stops;
        for (const auto &[name_stop, coordinates] : stops_)
        {
            if (!FindStopInformation(name_stop).buses.empty())
            {
                stops[name_stop] = &coordinates;
            }
        }
        return stops;
    }

    std::map<std::string_view, const Bus *> TransportCatalogue::FindAllWorkingBuses() const
    {
        std::map<std::string_view, const Bus *> buses;
        for (const auto &[name_bus, bus_info] : buses_)
        {
            if (!bus_info.stops.empty())
            {
                buses[name_bus] = &bus_info;
            }
        }
        return buses;
    }

    const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> &TransportCatalogue::GetDistanceBetweenStops() const
    {
        return distance_between_stops_;
    }

    const std::unordered_map<std::string, Bus> &TransportCatalogue::GetAllBuses() const
    {
        return buses_;
    }

    const std::unordered_map<std::string, geo::Coordinates> &TransportCatalogue::GetAllStops() const
    {
        return stops_;
    }

    std::tuple<int, int, double, double> TransportCatalogue::CalculateBusInformation(const Bus &bus) const
    {
        std::unordered_set<std::string_view> unique_stops;
        int stops_on_route = 0;
        double straight_route_length = 0;
        double real_route_length = 0;
        if (bus.stops.size() != 0)
        {
            for (size_t i = 0; i < bus.stops.size() - 1; ++i)
            {
                unique_stops.insert(bus.stops[i].first);
                straight_route_length += ComputeDistance(*bus.stops[i].second, *bus.stops[i + 1].second);
                real_route_length += CalculateDistance(bus.stops[i], bus.stops[i + 1]);
            }
            unique_stops.insert(bus.stops.back().first);
            stops_on_route = bus.stops.size();
            if (!bus.is_circular)
            {
                straight_route_length *= 2;
                stops_on_route = stops_on_route * 2 - 1;
                for (int i = bus.stops.size() - 1; i > 0; --i)
                {
                    real_route_length += CalculateDistance(bus.stops[i], bus.stops[i - 1]);
                }
            }
        }
        return {stops_on_route, unique_stops.size(), real_route_length, real_route_length / straight_route_length};
    }

    double TransportCatalogue::CalculateDistance(const std::pair<std::string_view, const geo::Coordinates *> stop_from,
                                                 const std::pair<std::string_view, const geo::Coordinates *> stop_to) const
    {
        double route_length = 0;
        if (distance_between_stops_.count(stop_from.first) && distance_between_stops_.at(stop_from.first).count(stop_to.first))
        {
            route_length += distance_between_stops_.at(stop_from.first).at(stop_to.first);
        }
        else if (distance_between_stops_.count(stop_to.first) && distance_between_stops_.at(stop_to.first).count(stop_from.first))
        {
            route_length += distance_between_stops_.at(stop_to.first).at(stop_from.first);
        }
        else
        {
            route_length = ComputeDistance(*stop_from.second, *stop_to.second);
        }
        return route_length;
    }
}