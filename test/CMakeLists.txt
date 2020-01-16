cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project(Test)

add_executable(Test
    "test.cpp"
)

set_property(TARGET Test
    PROPERTY CXX_STANDARD 17
)
