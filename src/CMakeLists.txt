set(DB_SOURCES DB/DB.cpp)
set(DB_INCLUDES DB/DB.h)

set(SERVER_SOURCES Server/Server.cpp)
set(SERVER_INCLUDES Server/Server.h)

add_library(GameEngine ${SOURCES} ${INCLUDES})
add_library(DBLib ${DB_SOURCES} ${DB_INCLUDES})
add_library(ServerLib ${SERVER_SOURCES} ${SERVER_INCLUDES})

add_executable(Potions main.cpp)
add_dependencies(Potions GameEngine ServerLib DBLib)
find_package(Boost COMPONENTS system REQUIRED)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(ServerLib OpenSSL::SSL)

target_link_libraries(DBLib mongo::mongocxx_shared)
target_link_libraries(ServerLib Threads::Threads Boost::boost)
target_link_libraries(Potions GameEngine ServerLib DBLib Boost::system)
