cmake_minimum_required(VERSION 2.8.4)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message("Debug Build")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    message("Release Build")
endif()
project(Potions)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
if (APPLE)
elseif(LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches \
        -Wlogical-op -Wuseless-cast")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -pedantic-errors -std=c++17 -DRAPIDJSON_HAS_STDSTRING=1 \
    -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align \
    -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion \
    -Wnull-dereference -Wdouble-promotion -Wformat=2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -O3 -ggdb3 -DRELEASE -DNDEBUG")
include_directories("${PROJECT_SOURCE_DIR}")
file(GLOB SOURCES "*.cpp")

add_executable(Potions ${SOURCES})
enable_testing()
add_subdirectory(tests)