enable_testing()

set(unity_root unity)
set(test_src
    tests.c
    test_runners/runner.c
    ${unity_root}/src/unity.c
    )

set(src
    ${CMAKE_SOURCE_DIR}/src/romfs.c
    ${CMAKE_SOURCE_DIR}/src/romfs-internal.c
    )

add_executable(tests ${test_src} ${src})
add_test(all_tests tests)

target_include_directories(tests PUBLIC ${unity_root}/src)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
