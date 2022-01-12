file(GLOB sources
    "./*.c"
)

add_library(romfs STATIC ${sources})
target_compile_features(romfs PRIVATE c_std_99)

option(TRACES "Enable debug traces" OFF)

target_include_directories(romfs PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (TRACES)
    message("-- Debug traces enabled")
    target_compile_definitions(romfs PUBLIC DEBUG=1)
endif()
