cmake_minimum_required(VERSION 3.17)
project(SIMPLE_HTTP_CLIENT)

find_package(stdnet REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)

add_executable(simple_http_client main.cpp)
target_link_libraries(simple_http_client stdnet::stdnet fmt::fmt)
