add_executable(Demo Demo.cpp)
target_link_libraries(Demo PRIVATE scope::scope)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
  target_link_libraries(Demo PRIVATE "stdc++fs")
endif()
