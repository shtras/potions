#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Game.h"

TEST_CASE("Initialization test", "[engine]")
{
    Game g;
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);

    auto elik_vern = g.GetCard(14);
    REQUIRE(elik_vern != nullptr);
    REQUIRE_FALSE(elik_vern->IsAssembled());
    REQUIRE(elik_vern->GetIngredient() == 7);
}

TEST_CASE("Assembling test", "[engine]")
{
    Game g;
    auto res = g.Init("../res/settings.json");
    REQUIRE(res);

    auto elik_vern = g.GetCard(14);  // requirements: i5 and i8
    auto elik_fire = g.GetCard(24);  // i12
    auto mand_root = g.GetCard(11);  // i5. Requires i7 and i6
    auto mand_root1 = g.GetCard(11); // i5
    auto drac_tooth = g.GetCard(17); // i8
    auto i7 = g.GetCard(45);
    auto i6 = g.GetCard(12);

    SECTION("Simple assemble")
    {
        std::set<Card*> parts = {mand_root, drac_tooth};
        REQUIRE(elik_vern->CanAssemble(parts));
    }

    SECTION("Wrong cards")
    {
        std::set<Card*> parts = {elik_fire, mand_root};
        REQUIRE_FALSE(elik_vern->CanAssemble(parts));
    }

    SECTION("Too many cards")
    {
        std::set<Card*> parts = {elik_fire, mand_root, drac_tooth};
        REQUIRE_FALSE(elik_vern->CanAssemble(parts));
    }

    SECTION("Too few cards")
    {
        std::set<Card*> parts = {drac_tooth};
        REQUIRE_FALSE(elik_vern->CanAssemble(parts));
    }

    SECTION("Same card")
    {
        std::set<Card*> parts = {mand_root, mand_root1};
        REQUIRE_FALSE(elik_vern->CanAssemble(parts));
    }

    SECTION("Recipe instead of ingredient")
    {
        std::set<Card*> parts = {i7, i6};
        REQUIRE(mand_root->CanAssemble(parts));
        mand_root->Assemble(parts);
        std::set<Card*> parts1 = {mand_root, drac_tooth};
        REQUIRE_FALSE(elik_vern->CanAssemble(parts1));
        mand_root->Disassemble();
        REQUIRE(elik_vern->CanAssemble(parts1));
    }
}
