#pragma once

#include <list>
#include <set>
#include <memory>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
namespace Engine
{
class Card;

class Requirement
{
public:
    enum class Type { None, Ingredient, Recipe };
    bool Parse(const rapidjson::Value& o);
    bool Matches(const Card* card) const;

private:
    Type type_ = Type::None;
    std::set<int> ids_ = {};
};

class Card
{
public:
    enum class Type { Recipe, Spell };
    bool Parse(int id, const rapidjson::Value& o);
    int GetIngredient() const;
    int GetScore() const;
    int GetID() const;
    bool CanAssemble(const std::set<Card*>& parts) const;
    bool IsAssembled() const;
    void Disassemble();
    void Assemble(std::set<Card*>& parts);

private:
    std::string name_ = "";
    Type type_ = Type::Recipe;
    bool assembled_ = false;
    std::set<int> ingredients_ = {};
    int id_ = -1;
    int score_ = 0;
    std::list<Requirement> requirements_ = {};
    std::set<Card*> assembledParts_ = {};
};
} // namespace Engine
