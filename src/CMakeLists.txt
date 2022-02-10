file(GLOB sources
    "./*.c"
)

add_library(${TARGET} STATIC ${sources})
target_compile_features(${TARGET} PRIVATE c_std_99)

option(ROMFS_DEBUG_TRACES "Enable debug traces" OFF)
set(ROMFS_MAX_PATH_LEN 256 CACHE STRING "Maximum path length")
set(ROMFS_MAX_FILE_NAME_LEN 32 CACHE STRING "Maximum file name length")

target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_definitions(${TARGET} PUBLIC
    MAX_PATH_LEN=${ROMFS_MAX_PATH_LEN}
    MAX_NAME_LEN=${ROMFS_MAX_FILE_NAME_LEN}
    PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
    )

if (ROMFS_DEBUG_TRACES)
    message("-- Debug traces enabled")
    target_compile_definitions(${TARGET} PUBLIC DEBUG=1)
endif()
