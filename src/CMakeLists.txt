set(sources
    "romfs.c"
    "romfs-internal.c"
)

add_library(romfs STATIC ${sources})
target_compile_features(romfs PRIVATE c_std_99)
