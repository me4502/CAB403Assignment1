cmake_minimum_required(VERSION 3.8)
project(CAB403Assignment1 C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES server/server.c client/client.c common/common.c common/map.c)

add_executable(Server server/server.c common/common.c common/map.c)
add_executable(Client client/client.c common/common.c common/map.c)