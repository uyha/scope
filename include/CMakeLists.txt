add_library(scope INTERFACE)
add_library(scope::scope ALIAS scope)

target_include_directories(scope INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(scope INTERFACE cxx_std_17)

