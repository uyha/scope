add_executable(scope_test scope17/src/Test.cpp ../scope.hpp)
target_include_directories(scope_test PRIVATE scope17/cute)
target_link_libraries(scope_test PRIVATE scope)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR
   ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows"))
    target_compile_definitions(scope_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast)
endif()

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(scope_test PRIVATE "stdc++fs")
endif()

add_test(NAME scope_test COMMAND scope_test)

