cmake_minimum_required(VERSION 3.9)
project(romfs-tool
    VERSION 0.1.0
    LANGUAGES C
    )

set(TARGET ${CMAKE_PROJECT_NAME})
set(VERSION ${CMAKE_PROJECT_VERSION})

message("Building ${TARGET} v${VERSION}")

add_executable(${TARGET} cmd/main.c)

add_subdirectory(${CMAKE_SOURCE_DIR}/src romfs)
add_subdirectory(${CMAKE_SOURCE_DIR}/test test)

target_include_directories(${TARGET} PUBLIC include)

target_link_libraries(${TARGET} PUBLIC romfs)

enable_testing()
