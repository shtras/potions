#include <iostream>

#include "spdlog_wrap.h"

#include "Engine/Game.h"
#include "DB/DB.h"
#include "Server/Server.h"

int main()
{
    spdlog::info("Welcome to spdlog!");
    auto& db = DB::DB::Instance();
    db.test();
    Server::Server s;
    s.Start();
    spdlog::info("Server is running. Press Enter to stop.");
    std::cin.get();
    s.Stop();
    spdlog::info("Bye");

    return 0;
}
