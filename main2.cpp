#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "json_reader.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr)
{
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv)
    {
        using Graph = graph::DirectedWeightedGraph<double>;
        proto_catalogue::TransportNavigator transport_navigator;
        reader::JsonReader json_data_base(std::cin);
        catalogue::TransportCatalogue transport_catalogue = json_data_base.CreateTransportCatalogue();
        *transport_navigator.mutable_catalogue() = serialization::CreateProtoCatalogue(transport_catalogue);
        catalogue::renderer::MapRenderer map_renderer(json_data_base.GetRenderSettings());
        *transport_navigator.mutable_render_settings() = serialization::CreateProtoRenderSettings(map_renderer.GetRenderSettings());
        catalogue::tr_router::TransoprtRouter transport_router(transport_catalogue, json_data_base.GetRoutingSettings());
        Graph graph = transport_router.CreateGraph();
        *transport_navigator.mutable_transport_router() = serialization::CreateProtoTransportRouter(transport_router);
        std::string output_file = json_data_base.GetSerializationSettings().AsDict().at("file"s).AsString();
        std::ofstream output(output_file, std::ios::binary);
        transport_navigator.SerializeToOstream(&output);
    }
    else if (mode == "process_requests"sv)
    {
        reader::JsonReader json_data_base(std::cin);
        std::string input_file = json_data_base.GetSerializationSettings().AsDict().at("file"s).AsString();
        std::ifstream input(input_file, std::ios::binary);
        proto_catalogue::TransportNavigator transport_navigator;
        transport_navigator.ParseFromIstream(&input);
        catalogue::TransportCatalogue transport_catalogue = serialization::DeserializeCatalogue(transport_navigator.catalogue());
        catalogue::renderer::MapRenderer map_renderer(serialization::DeserializeMapRenderer(transport_navigator.render_settings()));
        catalogue::tr_router::TransoprtRouter transport_router(transport_catalogue);
        using Graph = graph::DirectedWeightedGraph<double>;
        Graph graph = serialization::DeserializeTransportRouter(transport_navigator.transport_router(), transport_router);
        graph::Router router(graph);
        handler::RequestHandler request_handler(transport_catalogue, map_renderer, transport_router, router);
        json::Array information_found = request_handler.FindInformation(json_data_base.GetStatRequest());
        json::Print(json::Document{json::Node{information_found}}, std::cout);
    }
    else
    {
        PrintUsage();
        return 1;
    }
}