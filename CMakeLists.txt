cmake_minimum_required (VERSION 3.12...3.19 FATAL_ERROR)
project(scope VERSION 0.9.0 LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(include)
add_subdirectory(examples)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(cevelop-workspace)
endif()

include(packaging)
