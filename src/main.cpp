#include "Game.h"

#include <iostream>

int main()
{
    Game g;
    g.Init("res/cards.json");
    std::cout << "Hello" << std::endl;
    return 0;
}