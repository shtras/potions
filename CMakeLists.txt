cmake_minimum_required(VERSION 2.8.4)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message("Debug Build")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    message("Release Build")
endif()

option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    enable_testing()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

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

project(Potions)
#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(SOURCES Game.cpp Card.cpp Closet.cpp Player.cpp Utils/Utils.cpp)
set(INCLUDES Game.h Card.h Closet.h Player.h Utils/Utils.h)

add_subdirectory(src)
if (${ENABLE_TESTS})
    add_subdirectory(tests)
endif()