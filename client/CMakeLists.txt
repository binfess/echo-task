cmake_minimum_required(VERSION 3.14)

project(echo-client)

set(CMAKE_CXX_STANDARD 11)

add_executable(echo-client main.cpp echowriter.cpp echowriter.hpp abstractworker.cpp abstractworker.hpp)

target_link_libraries (echo-client LINK_PUBLIC netutils pthread)
