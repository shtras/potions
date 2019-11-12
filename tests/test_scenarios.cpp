#include "catch.hpp"
#include "Engine/Game.h"
#include "Utils/Utils.h"

TEST_CASE("Playing whirpool", "[scenarios]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto file = Utils::ReadFile("../res/testgame1.json");
    res = g.Parse(file);
    REQUIRE(res);
}
