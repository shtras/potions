add_library(GameEngine ${SOURCES} ${INCLUDES})

add_executable(Potions main.cpp)
add_dependencies(Potions GameEngine)
if(WIN32)
	find_package(Boost 1.56.0 REQUIRED)
endif()
find_package(mongocxx REQUIRED)

#target_link_libraries(Potions PRIVATE Boost::boost)
target_link_libraries(Potions PRIVATE mongo::mongocxx_shared)
target_link_libraries(Potions PRIVATE GameEngine)
