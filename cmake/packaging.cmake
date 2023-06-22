install(
  TARGETS scope
  EXPORT scopeTargets
  INCLUDES DESTINATION include
  )

install(
  EXPORT scopeTargets
  FILE scopeTargets.cmake
  NAMESPACE scope::
  DESTINATION lib/cmake/scope
  )

export(
  TARGETS scope
  NAMESPACE scope::
  FILE scopeTargets.cmake
  )

install(FILES include/scope.hpp DESTINATION include)

include(CMakePackageConfigHelpers)
configure_package_config_file(scopeConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/scopeConfig.cmake
  INSTALL_DESTINATION lib/cmake/scope
  )
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/scopeConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
  )

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/scopeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/scopeConfigVersion.cmake
  DESTINATION lib/cmake/scope
  )
