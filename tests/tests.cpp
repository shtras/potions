#define CATCH_CONFIG_MAIN
#include "spdlog_wrap.h"
#include "catch.hpp"
#include "Engine/Game.h"

TEST_CASE("Initialization test", "[engine]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    REQUIRE_NOTHROW(g.Prepare());
    auto w = g.GetWorld();

    auto r14 = w->GetCard(14);
    REQUIRE(r14 != nullptr);
    REQUIRE_FALSE(r14->IsAssembled());
    REQUIRE(r14->GetIngredient() == 7);
}

TEST_CASE("Assembling test", "[engine]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    auto w = g.GetWorld();

    auto r14 = w->GetCard(14); // requirements: i5 and i8
    auto i12 = w->GetCard(24);
    auto i4 = w->GetCard(8);
    auto i5 = w->GetCard(11); // rastv_vechn. Requires i7 and i6
    auto i5_1 = w->GetCard(11);
    auto i8 = w->GetCard(17);
    auto i7 = w->GetCard(45);
    auto i6 = w->GetCard(12);
    auto r49 = w->GetCard(49); // Requirements: 6,7 and 14,15
    auto r6 = w->GetCard(6);   // req: i4, i5

    SECTION("Simple assemble")
    {
        std::set<Engine::Card*> parts = {i5, i8};
        REQUIRE(r14->CanAssemble(parts));
    }

    SECTION("Wrong cards")
    {
        std::set<Engine::Card*> parts = {i12, i5};
        REQUIRE_FALSE(r14->CanAssemble(parts));
    }

    SECTION("Too many cards")
    {
        std::set<Engine::Card*> parts = {i12, i5, i8};
        REQUIRE_FALSE(r14->CanAssemble(parts));
    }

    SECTION("Too few cards")
    {
        std::set<Engine::Card*> parts = {i8};
        REQUIRE_FALSE(r14->CanAssemble(parts));
    }

    SECTION("Same card")
    {
        std::set<Engine::Card*> parts = {i5, i5_1};
        REQUIRE_FALSE(r14->CanAssemble(parts));
    }

    SECTION("Recipe instead of ingredient")
    {
        std::set<Engine::Card*> parts = {i7, i6};
        REQUIRE(i5->CanAssemble(parts));
        i5->Assemble(parts);
        std::set<Engine::Card*> parts_for_r14 = {i5, i8};
        REQUIRE_FALSE(r14->CanAssemble(parts_for_r14));
        i5->Disassemble();
        REQUIRE(r14->CanAssemble(parts_for_r14));
    }

    SECTION("Complex recipe")
    {
        std::set<Engine::Card*> parts_for_r6 = {i4, i5};
        std::set<Engine::Card*> parts_for_r14 = {i5, i8};
        std::set<Engine::Card*> parts_for_r49 = {r6, r14};
        REQUIRE_FALSE(r49->CanAssemble(parts_for_r49));
        r6->Assemble(parts_for_r6);
        REQUIRE_FALSE(r49->CanAssemble(parts_for_r49));
        r14->Assemble(parts_for_r14);
        REQUIRE(r49->CanAssemble(parts_for_r49));
    }
}

TEST_CASE("Parsing game state", "[engine]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    std::string gameState = "{"
                            "\"name\": \"Test game\","
                            "\"turn\": \"abcd\","
                            "\"state\": \"drawing\","
                            "\"closet\": {"
                            "    \"5\": [10, 44],"
                            "    \"8\": [16],"
                            "    \"9\": [18, 46, 60]"
                            "},"
                            "\"players\": ["
                            "    {"
                            "        \"id\": \"abcd\","
                            "        \"score\": 15,"
                            "        \"hand\": [27, 32, 33, 1, 7, 15],"
                            "        \"table\": ["
                            "            {"
                            "                \"17\": [67, 30],"
                            "                \"31\": [24, 20]"
                            "            }"
                            "        ]"
                            "    },"
                            "    {"
                            "        \"id\": \"defg\","
                            "        \"score\": 25,"
                            "        \"hand\": [37,38,39,40],"
                            "        \"table\": {"
                            "            \"8\": [6, 63]"
                            "        }"
                            "    }"
                            "]}";
    res = g.FromJson(gameState);
    REQUIRE(res);
}

TEST_CASE("Parsing moves", "[engine]")
{
    spdlog::set_level(spdlog::level::off);
    Engine::Move m;

    SECTION("Bad action")
    {
        bool res = m.Parse("{\"action\": \"bla\"}");
        REQUIRE_FALSE(res);
    }

    SECTION("No action")
    {
        bool res = m.Parse("{\"shmaction\": \"bla\"}");
        REQUIRE_FALSE(res);
    }

    SECTION("Draw move")
    {
        bool res = m.Parse("{\"action\": \"draw\"}");
        REQUIRE(res);
    }

    SECTION("Skip move")
    {
        bool res = m.Parse("{\"action\": \"skip\"}");
        REQUIRE(res);
    }

    SECTION("Assemble move")
    {
        bool res = m.Parse("{\"action\": \"assemble\", \"card\": 14,"
                           "\"parts\": ["
                           "   {\"type\": \"ingredient\", \"id\": 8},"
                           "   {\"type\": \"ingredient\", \"id\": 26}"
                           "]}");
        REQUIRE(res);
    }

    SECTION("Assemble move with bad card")
    {
        bool res = m.Parse("{\"action\": \"assemble\", \"card\": \"14\","
                           "\"parts\": ["
                           "   {\"type\": \"ingredient\", \"id\": 8},"
                           "   {\"type\": \"ingredient\", \"id\": 26}"
                           "]}");
        REQUIRE_FALSE(res);
    }

    SECTION("Assemble move with bad part types")
    {
        bool res = m.Parse("{\"action\": \"assemble\", \"card\": 14,"
                           "\"parts\": ["
                           "   {\"type\": \"spell\", \"id\": 8},"
                           "   {\"type\": \"ingredient\", \"id\": 26}"
                           "]}");
        REQUIRE_FALSE(res);
    }

    SECTION("Cast move")
    {
        bool res = m.Parse("{\"action\": \"cast\"}");
        REQUIRE(res);
    }
}
