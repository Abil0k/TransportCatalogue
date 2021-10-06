#include <iostream>
#include <sstream>

#include "json_reader.h"
#include "json_builder.h"
#include "transport_router.h"

int main()
{
    using Graph = graph::DirectedWeightedGraph<double>;

    reader::JsonReader json_data_base(std::cin);
    catalogue::TransportCatalogue transport_catalogue = json_data_base.CreateTransportCatalogue();
    catalogue::renderer::MapRenderer map_renderer(json_data_base.GetRenderSettings());
    catalogue::tr_router::TransoprtRouter transport_router(transport_catalogue, json_data_base.GetRoutingSettings());
    Graph graph = transport_router.CreateGraph();
    graph::Router router(graph);
    handler::RequestHandler request_handler(transport_catalogue, map_renderer, transport_router, router);
    json::Array information_found = request_handler.FindInformation(json_data_base.GetStatRequest());
    json::Print(json::Document{json::Node{information_found}}, std::cout);
}