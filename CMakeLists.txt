cmake_minimum_required(VERSION 3.10)

project(engine)

set(CMAKE_CXX_STANDARD 11)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(engine main.c)

target_link_libraries(engine  
                      PRIVATE SDL3::SDL3
                      PRIVATE m)
