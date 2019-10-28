#include <iostream>

#include "spdlog_wrap.h"

#include "Engine/Game.h"
#include "DB/DB.h"
#include "Server/Server.h"

int main()
{
    spdlog::info("Welcome to spdlog!");
    DB::A a;
    a.test();

    Server::Server s;
    s.Start();
    sleep(1000);
    Engine::Game g;
    g.Init("res/cards.json");
    return 0;
}
