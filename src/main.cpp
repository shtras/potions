#include "Game.h"

#include "spdlog_wrap.h"

#include <iostream>

int main()
{
    spdlog::info("Welcome to spdlog!");

    Game g;
    g.Init("res/cards.json");
    return 0;
}
