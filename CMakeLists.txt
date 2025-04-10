cmake_minimum_required(VERSION 3.10)
project(Chess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB SRC_FILES "src/*.cpp")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(chess ${SRC_FILES})

target_link_libraries(chess PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)