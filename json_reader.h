#pragma once

#include "request_handler.h"

namespace reader
{
    class JsonReader
    {
    public:
        JsonReader(std::istream &input);

        catalogue::TransportCatalogue CreateTransportCatalogue();

        const json::Node &GetStatRequest() const;

        const json::Node &GetRenderSettings() const;

        const json::Node &GetRoutingSettings() const;

        const json::Node &GetSerializationSettings() const;

    private:
        json::Node json_data_base_;

        domain::StopInputInfo ReadStopInputInfo(const json::Node &request);

        domain::StopInputInfo ReadDistanceInputInfo(const json::Node &request);

        domain::BusInputInfo ReadBusInputInfo(const json::Node &request);
    };
}