#define CATCH_CONFIG_MAIN
#include "spdlog_wrap.h"
#include "catch.hpp"
#include "Engine/Game.h"

TEST_CASE("Initialization test", "[engine]")
{
    Engine::Game g;
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    REQUIRE_NOTHROW(g.Prepare(3));

    auto r14 = g.GetCard(14);
    REQUIRE(r14 != nullptr);
    REQUIRE_FALSE(r14->IsAssembled());
    REQUIRE(r14->GetIngredient() == 7);
}

TEST_CASE("Assembling test", "[engine]")
{
    Engine::Game g;
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);

    auto r14 = g.GetCard(14); // requirements: i5 and i8
    auto i12 = g.GetCard(24);
    auto i4 = g.GetCard(8);
    auto i5 = g.GetCard(11); // rastv_vechn. Requires i7 and i6
    auto i5_1 = g.GetCard(11);
    auto i8 = g.GetCard(17);
    auto i7 = g.GetCard(45);
    auto i6 = g.GetCard(12);
    auto r49 = g.GetCard(49); // Requirements: 6,7 and 14,15
    auto r6 = g.GetCard(6);   // req: i4, i5

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
