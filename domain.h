#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

#include "geo.h"

namespace domain
{
    struct BusInputInfo
    {
        std::string name_bus;
        bool is_circular = false;
        std::vector<std::string> stops;
    };

    struct StopInputInfo
    {
        std::string name_stop;
        geo::Coordinates coordinates;
        std::unordered_map<std::string, int> distance_to_other_stops;
    };

    struct Bus
    {
        std::vector<std::pair<std::string_view, const geo::Coordinates *>> stops;
        bool is_circular = false;
    };

    struct BusInformation
    {
        std::string_view name_bus;
        int stops_on_route = 0;
        int unique_stops = 0;
        int route_length = 0;
        double curvature = 0;
    };

    struct StopInformation
    {
        std::string_view name_stop;
        std::set<std::string_view> buses;
    };

    struct EdgeInfo
    {
        std::string_view name_bus;
        int span_count = 0;
        double time = 0;
        std::string_view stop_from;
        std::string_view stop_to;
    };

    struct RouteInformation
    {
        double total_time = 0;
        std::vector<EdgeInfo> edges_info;
        double bus_wait_time = 0;
        bool route_found = false;
    };
}