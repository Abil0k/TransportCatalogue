cmake_minimum_required(VERSION 3.10)
project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto svg.proto transport_router.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} main1.cpp geo.cpp json_reader.cpp json.cpp map_renderer.cpp request_handler.cpp svg.cpp transport_catalogue.cpp json_builder.cpp transport_router.cpp transport_router.cpp serialization.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -g -Wall")

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


TARGET_LINK_LIBRARIES(transport_catalogue ${Protobuf_LIBRARIES})