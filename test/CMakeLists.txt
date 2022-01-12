enable_testing()

add_subdirectory(unity)

set(test_src
    tests.c
    test_runners/runner.c
    )

add_executable(tests ${test_src} ${src})
target_link_libraries(tests unity romfs)

add_test(all_tests tests)

target_include_directories(tests PUBLIC ${unity_root}/src)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
