set(DB_SOURCES DB/DB.cpp)
set(DB_INCLUDES DB/DB.h)

set(SERVER_SOURCES Server/Server.cpp)
set(SERVER_INCLUDES Server/Server.h)

add_library(GameEngine ${SOURCES} ${INCLUDES})
add_library(DBLib ${DB_SOURCES} ${DB_INCLUDES})

add_executable(Potions main.cpp ${SERVER_SOURCES} ${SERVER_INCLUDES})
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

target_link_libraries(GameEngine mongo::bsoncxx_shared)
target_link_libraries(DBLib mongo::mongocxx_shared)
target_link_libraries(Potions GameEngine DBLib Threads::Threads OpenSSL::SSL Boost::boost Boost::system)
