cmake_minimum_required (VERSION 3.12...3.19 FATAL_ERROR)

project(scope17 VERSION 0.9.0 LANGUAGES CXX)

set(MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MASTER_PROJECT ON)
  message(STATUS "CMake version: ${CMAKE_VERSION}")
endif()

#----------------------------------------------------------
# Compiler config
#----------------------------------------------------------
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_library(scope INTERFACE)
add_library(std::scope ALIAS scope)

### target_sources(scope INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/scope.hpp)
target_include_directories(scope INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_include_directories(scope SYSTEM INTERFACE
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/experimental>
)
target_compile_features(scope INTERFACE cxx_std_17)

option(SCOPE_TEST "Generate the scope_test target" ${MASTER_PROJECT})
if(SCOPE_TEST)
  enable_testing()
  add_subdirectory(cevelop-workspace)
endif()

option(SCOPE_INSTALL "Generate the scope_test target" ${MASTER_PROJECT})
if(SCOPE_INSTALL)
  install(TARGETS scope EXPORT scopeTargets
    INCLUDES DESTINATION include
  )

  install(EXPORT scopeTargets
    FILE scopeTargets.cmake
    NAMESPACE std::
    DESTINATION lib/cmake
  )

  export(TARGETS scope
    NAMESPACE std::
    FILE scopeTargets.cmake
  )

  install(FILES scope.hpp DESTINATION include/experimental)

  include(CMakePackageConfigHelpers)
  configure_package_config_file(scopeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/scopeConfig.cmake
    INSTALL_DESTINATION lib/cmake
  )
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/scopeConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
  )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scopeConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/scopeConfigVersion.cmake
      DESTINATION lib/cmake
  )
endif()
