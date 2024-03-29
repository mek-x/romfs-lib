option(UNITY_EXTENSION_FIXTURE "Compiles Unity with the \"fixture\" extension." ON)
add_subdirectory(unity)

file(GLOB test_src "./*.c")

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/generate_runner.sh
             ${CMAKE_CURRENT_BINARY_DIR}/runner.c
)

add_executable(tests ${test_src} ${CMAKE_CURRENT_BINARY_DIR}/runner.c)
target_link_libraries(tests unity romfs)

execute_process(
    COMMAND grep TEST_GROUP ${CMAKE_CURRENT_BINARY_DIR}/runner.c
    COMMAND bash "-c" "sed -e 's/.*RUN_TEST_GROUP(\\(.*\\));/\\1/'"
    COMMAND xargs
    OUTPUT_VARIABLE TEST_LIST
 )

string (REPLACE " " ";" TEST_LIST "${TEST_LIST}")
string (REPLACE "\n" "" TEST_LIST "${TEST_LIST}")

foreach(TEST_NAME ${TEST_LIST})
    message("-- Testing: adding test group \"${TEST_NAME}\"")
    add_test(${TEST_NAME} tests -g ${TEST_NAME})
endforeach()

target_include_directories(tests PRIVATE ${unity_root}/src)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
