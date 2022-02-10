cmake_minimum_required(VERSION 3.9)

include(cmake/VersionFromGit.cmake)

version_from_git(
    TIMESTAMP "%Y%m%d%H%M%S"
    )

project(romfs
    VERSION ${VERSION}
    LANGUAGES C
    )

include(CTest)

set(TARGET ${CMAKE_PROJECT_NAME})
set(VERSION ${CMAKE_PROJECT_VERSION})

message("Building ${TARGET} v${VERSION}")

add_executable(${TARGET}-tool cmd/main.c)

add_subdirectory(${CMAKE_SOURCE_DIR}/src ${TARGET})

target_link_libraries(${TARGET}-tool PUBLIC ${TARGET})

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test test)
endif()
