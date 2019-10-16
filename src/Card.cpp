#include <algorithm>

#include "Card.h"

bool Requirement::Matches(const Card* card) const
{
    auto cardType = card->GetRecipeType();
    if (cardType != type_) {
        return false;
    }
    switch (type_) {
        case Type::Ingredient:
            return card->GetIngredient() == id_;
        case Type::Potion:
            return card->IsAssembled() && card->GetRecipeId() == id_;
        case Type::GreatPotion:
            return card->IsAssembled() && card->GetRecipeType() == Type::GreatPotion;
        case Type::Talisman:
            return card->IsAssembled() && card->GetRecipeType() == Type::Talisman;
        case Type::Critter:
            return false;
        default:
            assert(0);
    }
    return false;
}

Card* AssemblePart::GetCard() const
{
    return card_;
}

bool Card::Parse(const rapidjson::Value::ConstObject& o)
{
    if (!o.HasMember("ingredient")) {
        return false;
    }
    return true;
}

int Card::GetIngredient() const
{
    if (ingredients_.size() != 1) {
        int res = 0;
        for (auto i : ingredients_) {
            res *= 100;
            res += i;
        }
        return res;
    }
    return *ingredients_.begin();
}

int Card::GetRecipeId() const
{
    return recipeId_;
}

int Card::GetScore() const
{
    return score_;
}

Requirement::Type Card::GetRecipeType() const
{
    return recipeType_;
}

bool Card::CanAssemble(std::set<AssemblePart*>& parts) const
{
    if (parts.size() != requirements_.size()) {
        return false;
    }
    for (const auto& r : requirements_) {
        auto found = std::find_if(parts.begin(), parts.end(), [&](const auto& p) { return r.Matches(p->GetCard()); });
        if (found == parts.end()) {
            return false;
        }
        parts.erase(found);
    }
    return true;
}

bool Card::IsAssembled() const
{
    return assembled_;
}

void Card::Disassemble()
{
    assembled_ = false;
    assembledParts_.clear();
}
