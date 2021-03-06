#pragma once

#include <list>
#include <set>
#include <memory>
#include <vector>

namespace Engine
{
class Card;

class Requirement
{
public:
    enum class Type { None, Ingredient, Recipe };
    bool Parse(const bsoncxx::document::view& d);
    bool Matches(const Card* card) const;
    Type GetType() const;

private:
    Type type_ = Type::None;
    std::set<int> ids_ = {};
};

class Card
{
public:
    enum class Type { Recipe, Spell };
    enum class TalismanType { None, Income, Generality, Growth, Usefulness };

    bool Parse(int id, const bsoncxx::document::view& d);
    int GetIngredient() const;
    bool HasIngredient(int id) const;
    int GetScore() const;
    int GetID() const;
    bool CanAssemble(const std::vector<Card*>& parts, int canSkipRequirements = 0) const;
    bool IsAssembled() const;
    bool HasPart(Card* c) const;
    void Disassemble(bool preserveState = false);
    void Assemble(const std::vector<Card*>& parts, bool usingUniversal = false);
    const std::set<Card*>& GetParts() const;
    Type GetType() const;
    bool UsingUniversal() const;
    TalismanType GetTalismanType() const;

private:
    std::string name_ = "";
    Type type_ = Type::Recipe;
    bool assembled_ = false;
    bool assembledUsingUniversal_ = false;
    std::set<int> ingredients_ = {};
    int id_ = -1;
    int score_ = 0;
    std::list<Requirement> requirements_ = {};
    std::set<Card*> assembledParts_ = {};
    TalismanType talismanType_ = TalismanType::None;
};
} // namespace Engine
