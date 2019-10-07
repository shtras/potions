add_library(PotionsLib ${SOURCES} ${INCLUDES})

add_executable(Potions main.cpp)
add_dependencies(Potions PotionsLib)
target_link_libraries(Potions PotionsLib)
