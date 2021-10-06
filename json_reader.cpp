#include <iostream>

#include "json_reader.h"

namespace reader
{
    using namespace std::string_literals;

    JsonReader::JsonReader(std::istream &input)
        : json_data_base_(json::Load(input).GetRoot())
    {
    }

    catalogue::TransportCatalogue JsonReader::CreateTransportCatalogue()
    {
        catalogue::TransportCatalogue transport_catalogue;
        auto base_requests = json_data_base_.AsDict().at("base_requests"s).AsArray();

        for (const auto &request : base_requests)
        {
            if (request.AsDict().at("type"s).AsString() == "Stop"s)
            {
                transport_catalogue.AddStop(ReadStopInputInfo(request));
            }
        }

        for (const auto &request : base_requests)
        {
            if (request.AsDict().at("type"s).AsString() == "Stop"s)
            {
                transport_catalogue.AddDistanceBetweenStop(ReadDistanceInputInfo(request));
            }
        }

        for (const auto &request : base_requests)
        {
            if (request.AsDict().at("type"s).AsString() == "Bus"s)
            {
                transport_catalogue.AddBus(ReadBusInputInfo(request));
            }
        }
        return transport_catalogue;
    }

    const json::Node &JsonReader::GetStatRequest() const
    {
        return json_data_base_.AsDict().at("stat_requests"s);
    }

    const json::Node &JsonReader::GetRenderSettings() const
    {
        return json_data_base_.AsDict().at("render_settings"s);
    }

    const json::Node &JsonReader::GetRoutingSettings() const
    {
        return json_data_base_.AsDict().at("routing_settings"s);
    }

    const json::Node &JsonReader::GetSerializationSettings() const
    {
        return json_data_base_.AsDict().at("serialization_settings"s);
    }

    domain::StopInputInfo JsonReader::ReadStopInputInfo(const json::Node &request)
    {
        domain::StopInputInfo stop_info;
        const auto &request_link = request.AsDict();
        stop_info.name_stop = request_link.at("name"s).AsString();
        stop_info.coordinates.lat = request_link.at("latitude"s).AsDouble();
        stop_info.coordinates.lng = request_link.at("longitude"s).AsDouble();
        return stop_info;
    }

    domain::StopInputInfo JsonReader::ReadDistanceInputInfo(const json::Node &request)
    {
        domain::StopInputInfo stop_info;
        const auto &request_link = request.AsDict();
        stop_info.name_stop = request_link.at("name"s).AsString();
        auto road_distances = request_link.at("road_distances"s).AsDict();
        for (const auto &[stop, distance] : road_distances)
        {
            stop_info.distance_to_other_stops[stop] = distance.AsInt();
        }
        return stop_info;
    }

    domain::BusInputInfo JsonReader::ReadBusInputInfo(const json::Node &request)
    {
        domain::BusInputInfo bus_info;
        const auto &request_link = request.AsDict();
        bus_info.name_bus = request_link.at("name"s).AsString();
        bus_info.is_circular = request_link.at("is_roundtrip"s).AsBool();
        auto stops = request_link.at("stops"s).AsArray();
        for (const auto &stop : stops)
        {
            bus_info.stops.push_back(stop.AsString());
        }
        return bus_info;
    }
}