#include "catch.hpp"
#include "Engine/Game.h"
#include "Utils/Utils.h"

TEST_CASE("Playing whirpool", "[scenarios]")
{
    REQUIRE(false);
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto file = Utils::ReadFile("../res/testgame77.json");
    res = g.Parse(file);
    REQUIRE(res);
    auto p1move = std::make_shared<Engine::Move>("player1");
    auto p2move = std::make_shared<Engine::Move>("player2");
    REQUIRE(p1move->Parse("{\"action\":\"cast\",\"card\":76}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse("{\"action\":\"cast\",\"card\":77}"));
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    REQUIRE(p1move->Parse("{\"action\":\"endturn\"}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse("{\"action\":\"draw\",\"deck\":\"base\"}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p2move->Parse("{\"action\":\"disassemble\",\"card\":8}"));
    REQUIRE(p1move->Parse("{\"action\":\"disassemble\",\"card\":17}"));
    REQUIRE_FALSE(g.ValidateMove(p2move.get()));
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse("{\"action\":\"disassemble\",\"card\":31}"));
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse("{\"action\":\"skip\"}"));
    REQUIRE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse("{\"action\":\"endturn\"}"));
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(g.ValidateMove(p2move.get()));
    g.PerformMove(p2move);
    REQUIRE(p2move->Parse("{\"action\":\"endturn\"}"));
    REQUIRE(g.ValidateMove(p2move.get()));
    g.PerformMove(p2move);
    bsoncxx::builder::stream::document d;
    g.ToJson(d);
    const auto& view = d.view();
    REQUIRE(std::string(view["state"].get_utf8().value) == "done");
    REQUIRE(std::string(view["turn"].get_utf8().value) == "player1");
    REQUIRE(
        std::string(view["specialstate"].get_document().value["state"].get_utf8().value) == "none");
    const auto& p1Table =
        view["players"].get_array().value[0].get_document().value["table"].get_document().value;
    const auto& p2Table =
        view["players"].get_array().value[1].get_document().value["table"].get_document().value;
    REQUIRE(Utils::BsonArraySize(p1Table["17"].get_array().value) == 0);
    REQUIRE(Utils::BsonArraySize(p1Table["31"].get_array().value) == 0);
    REQUIRE(Utils::BsonArraySize(p2Table["8"].get_array().value) == 0);
    // 77  67, 30, 24, 20, 6, 63
    //  1  6   15  12  10  3  7
    const auto& closet = view["closet"].get_document().value;
    REQUIRE(Utils::BsonArraySize(closet["1"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["3"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["6"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["7"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["10"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["12"].get_array().value) == 1);
    REQUIRE(Utils::BsonArraySize(closet["15"].get_array().value) == 4);
}

TEST_CASE("Playing necessity", "[scenarios]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto file = Utils::ReadFile("../res/testgame80.json");
    res = g.Parse(file);
    REQUIRE(res);
    auto p1move = std::make_shared<Engine::Move>("player1"); // 13, 11, 15, 0, 3, 7, [14]
    auto p2move = std::make_shared<Engine::Move>("player2"); // 11, 15, 13, 13
    auto p3move = std::make_shared<Engine::Move>("player3"); // 13, 12, 12, 5
    p1move->Parse("{\"action\":\"cast\",\"card\":80,\"ingredient\":3}");
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    p1move->Parse("{\"action\":\"discard\",\"card\":1}");
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    p2move->Parse("{\"action\":\"discard\",\"card\":37}");
    REQUIRE_FALSE(g.ValidateMove(p2move.get()));
    p3move->Parse("{\"action\":\"discard\",\"card\":41}");
    REQUIRE_FALSE(g.ValidateMove(p3move.get()));
    p2move->Parse("{\"action\":\"skip\"}");
    REQUIRE(g.ValidateMove(p2move.get()));
    g.PerformMove(p2move);
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE_FALSE(g.ValidateMove(p2move.get()));
    REQUIRE_FALSE(g.ValidateMove(p3move.get()));
    p3move->Parse("{\"action\":\"skip\"}");
    REQUIRE(g.ValidateMove(p3move.get()));
    g.PerformMove(p3move);
    p1move->Parse("{\"action\":\"discard\",\"card\":7}");
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    bsoncxx::builder::stream::document d;
    g.ToJson(d);
    const auto& view = d.view();
    REQUIRE(std::string(view["state"].get_utf8().value) == "playing");
    REQUIRE(std::string(view["turn"].get_utf8().value) == "player1");
    REQUIRE(
        std::string(view["specialstate"].get_document().value["state"].get_utf8().value) == "none");
}

TEST_CASE("Playing global reveal", "[scenarios]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto file = Utils::ReadFile("../res/testgame83.json");
    res = g.Parse(file);
    REQUIRE(res);
    auto p1move = std::make_shared<Engine::Move>("player1");
    auto p2move = std::make_shared<Engine::Move>("player2");
    p1move->Parse("{\"action\":\"cast\",\"card\":83}");
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse(
        "{\"action\":\"cast\",\"card\":83,\"parts\":[{\"type\":\"ingredient\",\"id\":56}]}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse(
        "{\"action\":\"cast\",\"card\":83,\"parts\":[{\"type\":\"ingredient\",\"id\":44},"
        "{\"type\":\"ingredient\",\"id\":118}]}"));
    REQUIRE_FALSE(g.ValidateMove(p1move.get()));
    REQUIRE(p1move->Parse(
        "{\"action\":\"cast\",\"card\":83,\"parts\":[{\"type\":\"ingredient\",\"id\":118}]}"));
    REQUIRE(g.ValidateMove(p1move.get()));
    g.PerformMove(p1move);
    bsoncxx::builder::stream::document d;
    g.ToJson(d);
    const auto& view = d.view();
    REQUIRE(std::string(view["state"].get_utf8().value) == "done");
    REQUIRE(std::string(view["turn"].get_utf8().value) == "player1");
}