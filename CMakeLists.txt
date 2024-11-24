cmake_minimum_required(VERSION 3.20)

project(SQLight)

add_library(sqlight SHARED sqlight.hpp sqlitght.cpp)
target_compile_features(sqlight PRIVATE cxx_std_23)
set_target_properties(sqlight PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(sqlight sqlite3)

add_executable(sqlight_test main.cpp)

target_compile_features(sqlight_test PRIVATE cxx_std_23)
set_target_properties(sqlight_test PROPERTIES CXX_EXTENSIONS OFF)
                                 
target_link_libraries(sqlight_test sqlight)
