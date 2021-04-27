
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/23ef29555ef4789f555f1ba8c51b4c52975f0907.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

file(GLOB TEST_SRC "*.cpp")
foreach(TEST ${TEST_SRC})
    string(REPLACE ".cpp" "" TEST_NAME_DIR ${TEST})
    get_filename_component(TEST_NAME ${TEST_NAME_DIR} NAME)
    add_executable(${TEST_NAME} ${TEST})
    target_link_libraries(${TEST_NAME} OGL gtest_main)
    add_dependencies(${TEST_NAME} OGL)
    gtest_discover_tests(${TEST_NAME})
    

endforeach(TEST ${TEST_SRC})

add_subdirectory(special)