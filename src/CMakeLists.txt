set(sources
    "romfs.c"
    "romfs-internal.c"
)

add_library(romfs STATIC ${sources})
target_compile_features(romfs PRIVATE c_std_99)

option(TRACES "Enable debug traces" OFF)

if (TRACES)
    message("-- Debug traces enabled")
    target_compile_definitions(romfs PUBLIC DEBUG)
endif()
