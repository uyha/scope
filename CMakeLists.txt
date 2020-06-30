
cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

enable_testing()

project(scope17 LANGUAGES CXX)

set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MASTER_PROJECT ON)
  message(STATUS "CMake version: ${CMAKE_VERSION}")
endif ()

option(SCOPE_TEST "Generate the scope_test target" ${MASTER_PROJECT})

if (SCOPE_TEST)
  add_subdirectory(cevelop-workspace)
endif ()

add_library(scope INTERFACE)
target_sources(scope INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/scope.hpp)
target_include_directories(scope INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_include_directories(scope SYSTEM INTERFACE
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/scope17>
)
target_compile_features(scope INTERFACE cxx_std_17)
