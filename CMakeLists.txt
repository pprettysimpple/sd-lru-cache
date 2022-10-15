cmake_minimum_required(VERSION 3.23)
project(LRUCache)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(googletest
        GIT_REPOSITORY  https://github.com/google/googletest
        GIT_TAG         main
        CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        )

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
include_directories(lib)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable(TestAll tests/test.cpp)
add_dependencies(TestAll googletest)
target_link_libraries(TestAll gtest gtest_main pthread)
add_compile_definitions(DEBUG=1)