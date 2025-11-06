cmake_minimum_required(VERSION 3.11)

set(CMAKE_C_STANDARD 99)
set(PROJECT_NAME bunshell)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode enabled")
    add_compile_options(-g -O0 -Wall -Wextra)
    
    # Optional: Enable sanitizers in debug mode
    option(ENABLE_ASAN "Enable AddressSanitizer" ON)
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

project(${PROJECT_NAME} C)

file(GLOB_RECURSE SOURCES "src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCES})
