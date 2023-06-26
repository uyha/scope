cmake_minimum_required (VERSION 3.12...3.19 FATAL_ERROR)
project(scope VERSION 1.1.0 LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(include)
add_subdirectory(examples)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

include(packaging)
