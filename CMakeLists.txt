cmake_minimum_required(VERSION 3.5)

project(mailClient_v_0_1_0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mailClient_v_0_1_0 main.cpp)

install(TARGETS mailClient_v_0_1_0
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
