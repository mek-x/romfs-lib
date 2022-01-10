cmake_minimum_required(VERSION 3.9)
project(romfs-tool)

set(TARGET ${CMAKE_PROJECT_NAME})
set(VERSION "0.0.1")

message("Building ${TARGET} v${VERSION}")

add_executable(${TARGET} cmd/main.c)

add_subdirectory(${CMAKE_SOURCE_DIR}/src romfs)

target_include_directories(${TARGET} PUBLIC include)

target_link_libraries(${TARGET} PUBLIC romfs)
