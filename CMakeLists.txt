
cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

enable_testing()

project(scope_test LANGUAGES CXX)

add_executable(scope_test cevelop-workspace/scope17/src/Test.cpp cevelop-workspace/scope17/src/scope.hpp)

target_include_directories(scope_test PRIVATE cevelop-workspace/scope17/cute)

set_property(TARGET scope_test PROPERTY CXX_STANDARD 17)

if(
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows"))
    target_compile_definitions(scope_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(scope_test PRIVATE "stdc++fs")
endif()

add_test(NAME scope_test COMMAND scope_test)
