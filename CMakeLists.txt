cmake_minimum_required(VERSION 3.5)

project(mailClient_v_0_1_0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(vmime REQUIRED)

set(CMAKE_PREFIX_PATH "/usr/include/gtk-2.0/gtk")
#find_package(gtk)

find_package(GTK2 2.6 REQUIRED gtk)



if(GTK2_FOUND)
    include_directories(${GTK2_INCLUDE_DIRS})
    add_executable(${PROJECT_NAME} main.cpp authenticator.hpp example6_certificateVerifier.hpp example6_timeoutHandler.hpp)
    target_link_libraries(${PROJECT_NAME} ${GTK2_LIBRARIES} vmime)
endif()

add_executable(mailClientTEST example6.cpp example6_authenticator.hpp
        example6_certificateVerifier.hpp example6_timeoutHandler.hpp
        example6_tracer.hpp)

target_link_libraries(mailClientTEST PRIVATE vmime)
