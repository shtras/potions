#include "catch.hpp"
#include "Engine/Game.h"
#include "Utils/Utils.h"

TEST_CASE("Playing assemble with Generality", "[scenarios]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto file = Utils::ReadFile("../res/testgame110.json");
    res = g.Parse(file);
    REQUIRE(res);
    auto p1move = std::make_shared<Engine::Move>("player1");
    auto p2move = std::make_shared<Engine::Move>("player2");
    REQUIRE(p1move->Parse("{\"action\":\"cast\",\"card\":77}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p2move->Parse("{\"action\":\"assemble\",\"card\":51,\"parts\":[{\"type\":\"recipe\","
                          "\"id\":30},{\"type\":\"recipe\",\"id\":12}]}"));
    REQUIRE(g.ValidateMove(p2move.get()));
    g.PerformMove(p2move);

    bsoncxx::builder::stream::document d;
    g.ToJson(d);
    const auto& view = d.view();
    auto p1score = view["players"].get_array().value[0].get_document().value["score"].get_int32();
    auto p2score = view["players"].get_array().value[1].get_document().value["score"].get_int32();
    REQUIRE(p1score == 21);
    REQUIRE(p2score == 31);
}