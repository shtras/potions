#pragma once

#include <list>
#include <set>
#include <memory>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

class Card;

class Requirement
{
public:
    enum class Type { None, Ingredient, Potion, GreatPotion, Talisman, Critter };
    bool Matches(const Card* card) const;

private:
    Type type_ = Type::None;
    int id_ = -1;
};

class AssemblePart
{
public:
    enum class Type { Ingredient, Assembled };

    Type GetType() const;
    Card* GetCard() const;

private:
    Type type_ = Type::Ingredient;
    Card* card_ = nullptr;
};

class Card
{
public:
    enum class Type { Recipe, Spell };
    void Parse(rapidjson::Value::Object& o);
    int GetIngredient() const;
    int GetRecipeId() const;
    Requirement::Type GetRecipeType() const;
    bool CanAssemble(std::set<AssemblePart*>& parts);
    bool IsAssembled() const;
    void Disassemble();
    void Assemble(std::set<Card*> parts);

private:
    Type type_ = Type::Recipe;
    bool assembled_ = false;
    Requirement::Type recipeType_ = Requirement::Type::None;
    std::set<int> ingredients_ = {};
    int recipeId_ = -1;
    std::set<Requirement> requirements_ = {};
    std::set<Card*> assembledParts_ = {};
};
