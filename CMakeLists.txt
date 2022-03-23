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

add_executable(cut src/main.c)