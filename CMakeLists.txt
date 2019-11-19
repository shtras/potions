project(Potions)
cmake_minimum_required(VERSION 3.8)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0") 
    cmake_policy(SET CMP0074 NEW)
endif()
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message("Debug Build")
    add_definitions(-DFORCE_HTTP)
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    message("Release Build")
endif()

option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    enable_testing()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRAPIDJSON_HAS_STDSTRING=1")
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /permissive- /W4 /w14640 /w14242 /w14254 /w14263 /w14265 /w14287 \
        /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14826 /w14905 /w14906 /w14928")
else()
    if (APPLE)
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches \
            -Wlogical-op -Wuseless-cast")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -pedantic-errors -std=c++17 \
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align \
        -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion \
        -Wnull-dereference -Wdouble-promotion -Wformat=2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -O3 -ggdb3")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(SOURCES Engine/Game.cpp Engine/World.cpp Engine/Card.cpp Engine/Closet.cpp Engine/Player.cpp Engine/Move.cpp Utils/Utils.cpp)
set(INCLUDES Engine/Game.h Engine/World.h Engine/Card.h Engine/Closet.h Engine/Player.h Engine/Move.h Engine/Ingredient.h spdlog_wrap.h bsoncxx_wrap.h Utils/Utils.h)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

add_subdirectory(src)
if (${ENABLE_TESTS})
    add_subdirectory(tests)
endif()
if (WIN32)
set_property(TARGET Potions unit_tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/run")
endif()
