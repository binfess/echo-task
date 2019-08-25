cmake_minimum_required(VERSION 3.14)

project(echo-server)

set(CMAKE_CXX_STANDARD 11)

add_executable(echo-server main.cpp echoserver.cpp echoserver.hpp)

target_link_libraries (echo-server LINK_PUBLIC netutils)
