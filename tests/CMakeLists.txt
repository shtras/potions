add_executable(unit_tests tests.cpp)
target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_dependencies(unit_tests PotionsLib)
target_link_libraries(unit_tests PotionsLib)

set(CMAKE_CXX_STANDARD 17)

add_test(NAME Test1 COMMAND unit_tests WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})