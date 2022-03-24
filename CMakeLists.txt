cmake_minimum_required(VERSION 3.10)

project(cpu-usage-tracker)
set(C_STANDARD 99)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Weverything)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra)
else()
  message(WARNING "Unsupported compiler ${CMAKE_C_COMPILER_ID}! Supported compilers are gcc and clang.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(EXE_NAME cut)
set(LIB_NAME cut-lib)
set(TEST_NAME cut-test)

include_directories(src)

add_library(${LIB_NAME}
    src/queue.c
    src/reader.c
    src/analyzer.c
    src/printer.c
    src/watchdog.c
)

add_executable(${EXE_NAME} 
    src/main.c
)

add_executable(${TEST_NAME}
    test/test.c
)

enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

target_link_libraries(${LIB_NAME} INTERFACE pthread)

target_link_libraries(${EXE_NAME} ${LIB_NAME})
target_link_libraries(${TEST_NAME} ${LIB_NAME})