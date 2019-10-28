#include <iostream>

#include "spdlog_wrap.h"

#include "Engine/Game.h"

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

int main()
{
    spdlog::info("Welcome to spdlog!");

    Engine::Game g;
    g.Init("res/cards.json");
    return 0;
}
