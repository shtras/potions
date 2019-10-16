#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Game.h"

TEST_CASE("Initialization test", "[engine]")
{
    Game g;
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
}