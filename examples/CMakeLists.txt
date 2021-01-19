cmake_minimum_required (VERSION 3.12...3.19 FATAL_ERROR)

project(demo LANGUAGES CXX)

if(NOT TARGET scope)
  find_package(scope CONFIG REQUIRED)
endif()

add_executable(Demo Demo.cpp)
target_link_libraries(Demo PRIVATE std::scope)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
  target_link_libraries(Demo PRIVATE "stdc++fs")
endif()
