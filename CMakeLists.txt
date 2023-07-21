cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(mailClient_v_0_1_0)

find_package(vmime REQUIRED)

add_executable(${PROJECT_NAME} main.cpp authenticator.hpp example6_certificateVerifier.hpp example6_timeoutHandler.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE vmime)

add_executable(mailClientTEST example6.cpp example6_authenticator.hpp
        example6_certificateVerifier.hpp example6_timeoutHandler.hpp
        example6_tracer.hpp)

target_link_libraries(mailClientTEST PRIVATE vmime)
