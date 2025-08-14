cmake_minimum_required(VERSION 3.31)

project(
    capy
    VERSION 0.1.0
    DESCRIPTION "capy"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

include_directories("include")

add_library(capy SHARED src/capy.c)


add_executable(smap_test tests/smap_test.c)
target_link_libraries(smap_test capy)
add_test(NAME smap_test COMMAND smap_test)

add_executable(vec_test tests/vec_test.c)
target_link_libraries(vec_test capy)
add_test(NAME vec_test COMMAND vec_test)

add_executable(symbols_test tests/symbols_test.c)
target_link_libraries(symbols_test capy)
add_test(NAME symbols_test COMMAND symbols_test)
