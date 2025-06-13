cmake_minimum_required(VERSION 3.20)

project(SQLight)

include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(sqlight STATIC include/sqlight.hpp src/sqlitght.cpp)

# target_compile_options(sqlight PUBLIC -g --coverage -fno-inline -fprofile-instr-generate -fcoverage-mapping)
# target_link_options(sqlight PUBLIC --coverage)

target_compile_features(sqlight PRIVATE cxx_std_23)
set_target_properties(sqlight PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(sqlight
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


target_link_libraries(sqlight sqlite3)

add_subdirectory(tests)

add_test(sqlight_test ${CMAKE_CURRENT_BINARY_DIR}/tests/sqlight_test)