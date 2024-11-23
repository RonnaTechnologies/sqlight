cmake_minimum_required(VERSION 3.20)

project(SQLight)


add_executable(sqlight_test main.cpp sqlitght.cpp)

target_compile_features(sqlight_test PRIVATE cxx_std_23)
set_target_properties(sqlight_test PROPERTIES CXX_EXTENSIONS OFF)
                                 
target_link_libraries(sqlight_test sqlite3)
