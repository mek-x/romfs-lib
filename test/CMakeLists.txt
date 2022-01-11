enable_testing()

set(unity_root unity)
set(src
    tests.c
    test_runners/runner.c
    ${unity_root}/src/unity.c
    )

add_executable(tests ${src})
add_test(all_tests tests)

target_include_directories(tests PUBLIC ${unity_root}/src)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
