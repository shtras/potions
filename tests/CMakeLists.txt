set(TEST_SERVER_SOURCES ${PROJECT_SOURCE_DIR}/src/Server/Server.cpp)
set(TEST_SERVER_INCLUDES ${PROJECT_SOURCE_DIR}/src/Server/Server.h)
add_definitions(-DFORCE_HTTP)
add_executable(unit_tests test_main.cpp test_server.cpp test_game.cpp test_scenarios.cpp ${TEST_SERVER_SOURCES} ${TEST_SERVER_INCLUDES})

target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_dependencies(unit_tests GameEngine DBLib)
target_link_libraries(unit_tests GameEngine DBLib OpenSSL::SSL Threads::Threads Boost::boost Boost::system)

set(CMAKE_CXX_STANDARD 17)

add_test(NAME unit_tests COMMAND unit_tests WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})