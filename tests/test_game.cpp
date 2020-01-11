#include "catch.hpp"
#include "Engine/Game.h"

TEST_CASE("Initialization test", "[engine]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    std::string p1 = "Player1";
    std::string p2 = "Player2";
    g.AddPlayer(p1);
    g.AddPlayer(p2);
    REQUIRE_NOTHROW(g.Start());
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

    auto i4 = w->GetCard(8);
    auto i5 = w->GetCard(11); // rastv_vechn. Requires i7 and i6
    auto i5_1 = w->GetCard(11);
    auto i6 = w->GetCard(12);
    auto i7 = w->GetCard(45);
    auto i8 = w->GetCard(17);
    auto i9 = w->GetCard(18);
    auto i10 = w->GetCard(57);
    auto i12 = w->GetCard(24);
    auto i11_12_15 = w->GetCard(69); // Ingredients: 11, 12, 15
    auto i16_1 = w->GetCard(137);
    auto i16_2 = w->GetCard(138);

    auto r6 = w->GetCard(6);   // req: i4, i5
    auto r14 = w->GetCard(14); // requirements: i5 and i8
    auto r37 = w->GetCard(37); // Requirements: 15, 10, 9
    auto r49 = w->GetCard(49); // Requirements: 6,7 and 14,15

    auto ru_1 = w->GetCard(119);
    auto ru_2 = w->GetCard(120);

    SECTION("Simple assemble")
    {
        REQUIRE(r14->CanAssemble({i5, i8}));
        REQUIRE(r14->CanAssemble({i8, i5}));
    }

    SECTION("Wrong cards")
    {
        REQUIRE_FALSE(r14->CanAssemble({i12, i5}));
    }

    SECTION("Too many cards")
    {
        REQUIRE_FALSE(r14->CanAssemble({i12, i5, i8}));
    }

    SECTION("Too few cards")
    {
        REQUIRE_FALSE(r14->CanAssemble({i8}));
    }

    SECTION("Same card")
    {
        REQUIRE_FALSE(r14->CanAssemble({i5, i5_1}));
    }

    SECTION("Recipe instead of ingredient")
    {
        REQUIRE(i5->CanAssemble({i7, i6}));
        i5->Assemble({i7, i6});
        REQUIRE_FALSE(r14->CanAssemble({i5, i8}));
        i5->Disassemble();
        REQUIRE(r14->CanAssemble({i5, i8}));
    }

    SECTION("Complex recipe")
    {
        REQUIRE_FALSE(r49->CanAssemble({r6, r14}));
        r6->Assemble({i4, i5});
        REQUIRE_FALSE(r49->CanAssemble({r6, r14}));
        REQUIRE_FALSE(r49->CanAssemble({r6, r14}));
        REQUIRE_FALSE(r49->CanAssemble({r14, r6}));
        r14->Assemble({i5, i8});
        REQUIRE(r49->CanAssemble({r6, r14}));
        REQUIRE(r49->CanAssemble({r14, r6}));
    }

    SECTION("Multi ingredients card")
    {
        REQUIRE(r37->CanAssemble({i9, i10, i11_12_15}));
        REQUIRE(r37->CanAssemble({i9, i11_12_15, i10}));
        REQUIRE(r37->CanAssemble({i11_12_15, i9, i10}));
    }

    SECTION("Universal ingredients")
    {
        REQUIRE(r14->CanAssemble({i5, i16_1}));
        REQUIRE(r14->CanAssemble({i16_1, i8}));
        ru_1->Assemble({i5, i6});
        REQUIRE_FALSE(r14->CanAssemble({i5, ru_1}));
        REQUIRE_FALSE(r14->CanAssemble({i16_1, i16_2}));
    }

    SECTION("Universal potions")
    {
        REQUIRE(ru_1->CanAssemble({i5, i6}));
        ru_1->Assemble({i5, i6});
        REQUIRE(ru_2->CanAssemble({i9, i10}));
        ru_2->Assemble({i9, i10});
        r6->Assemble({i4, i5});
        r14->Assemble({i5, i8});
        REQUIRE(r49->CanAssemble({r6, ru_1}));
        REQUIRE(r49->CanAssemble({ru_1, r14}));
        REQUIRE_FALSE(r49->CanAssemble({ru_1, ru_2}));
        REQUIRE_FALSE(r49->CanAssemble({r6, i16_1}));
    }
}

TEST_CASE("Parsing game state", "[engine]")
{
    Engine::Game g("test game");
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);
    std::string gameState =
        "{\"state\":{"
        "\"name\": \"Test game\","
        "\"turn\": \"abcd\","
        "\"state\": \"drawing\",\"extraplaymoves\":0,"
        "\"closet\": {"
        "    \"5\": [10, 44],"
        "    \"8\": [16],"
        "    \"9\": [18, 46, 60]"
        "},\"specialstate\":{\"state\":\"none\",\"player\":0,\"drawremains\":0,\"ingredient\":-1},"
        "\"decks\": {\"base\":[1, 2, 3]},"
        "\"players\": ["
        "    {"
        "        \"user\": \"abcd\","
        "        \"score\": 15,"
        "        \"hand\": [27, 32, 33, 1, 7, 15],"
        "        \"table\": {"
        "            \"17\": {\"parts\":[67, 30]},"
        "            \"31\": {\"parts\":[24, 20]}"
        "        }"
        "    },"
        "    {"
        "        \"user\": \"defg\","
        "        \"score\": 25,"
        "        \"hand\": [37,38,39,40],"
        "        \"table\": {"
        "            \"8\": {\"parts\":[6, 63]}"
        "        }"
        "    }"
        "]},\"moves\":[],\"expansions\":0}";
    res = g.Parse(gameState);
    REQUIRE(res);
}

TEST_CASE("Parsing moves", "[engine]")
{
    spdlog::set_level(spdlog::level::off);
    std::string player = "Player1";
    Engine::Move m(player);

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
        bool res = m.Parse("{\"action\":\"draw\",\"deck\":\"base\"}");
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
        bool res = m.Parse("{\"action\": \"cast\", \"card\": 76, \"parts\": [{\"id\": 12, "
                           "\"type\": \"recipe\"}]}");
        REQUIRE(res);
    }
}