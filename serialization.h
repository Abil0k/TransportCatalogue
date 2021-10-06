#pragma once

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

#include "transport_router.h"
#include "map_renderer.h"

#include <fstream>

namespace serialization
{
    proto_catalogue::TransportCatalogue CreateProtoCatalogue(const catalogue::TransportCatalogue &transport_catalogue);

    proto_svg::Color GetProtoColor(const svg::Color &color);

    proto_map_renderer::RenderSettings CreateProtoRenderSettings(const catalogue::renderer::MapRenderer::RenderSettings &render_settings);

    proto_tr_router::TransportRouter CreateProtoTransportRouter(const catalogue::tr_router::TransoprtRouter &transport_router);

    catalogue::TransportCatalogue DeserializeCatalogue(const proto_catalogue::TransportCatalogue &proto_catalogue);

    svg::Color GetColor(proto_svg::Color proto_color);

    catalogue::renderer::MapRenderer::RenderSettings DeserializeMapRenderer(const proto_map_renderer::RenderSettings &proto_render_settings);

    using Graph = graph::DirectedWeightedGraph<double>;

    Graph DeserializeTransportRouter(const proto_tr_router::TransportRouter &proto_router, catalogue::tr_router::TransoprtRouter &tr_router);

}