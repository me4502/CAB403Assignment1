cmake_minimum_required(VERSION 3.7)
project(CAB403Assignment1 C)

set(CMAKE_C_STANDARD 11)

find_package(Threads)
set(SOURCE_FILES server/server.c client/client.c common/common.c common/map.c common/list.c)

add_executable(Server server/server.c common/common.c common/map.c common/list.c)
add_executable(Client client/client.c common/common.c common/map.c common/list.c)
add_executable(Test test/test.c common/common.c common/map.c common/list.c)
target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})