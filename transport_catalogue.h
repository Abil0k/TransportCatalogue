#pragma once

#include <unordered_set>
#include <tuple>
#include <map>

#include "domain.h"

namespace catalogue
{
    using namespace domain;
    class TransportCatalogue
    {
    public:
        void AddStop(StopInputInfo stop_info);

        void AddDistanceBetweenStop(const StopInputInfo &stop_info);

        void AddBus(const BusInputInfo &bus_info);

        BusInformation FindBusInformation(const std::string &query) const;

        StopInformation FindStopInformation(const std::string &query) const;

        std::map<std::string_view, const geo::Coordinates *> FindAllWorkingStops() const;

        std::map<std::string_view, const Bus *> FindAllWorkingBuses() const;

        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> &GetDistanceBetweenStops() const;

        const std::unordered_map<std::string, Bus> &GetAllBuses() const;

        const std::unordered_map<std::string, geo::Coordinates> &GetAllStops() const;

        double CalculateDistance(const std::pair<std::string_view, const geo::Coordinates *> stop_from,
                                 const std::pair<std::string_view, const geo::Coordinates *> stop_to) const;

    private:
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, geo::Coordinates> stops_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> buses_passing_stops_;
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distance_between_stops_;

        std::tuple<int, int, double, double> CalculateBusInformation(const Bus &bus) const;
    };
}