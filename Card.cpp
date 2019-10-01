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
            break;
        case Type::Potion:
            return card->GetRecipeId() == id_;
            break;
        case Type::GreatPotion:
            return card->GetRecipeType() == Type::GreatPotion;
        case Type::Talisman:
            return card->GetRecipeType() == Type::Talisman;
        case Type::Critter:
            return false;
        default:
            assert(0);
    }
    return false;
}

void Card::Parse(rapidjson::Value::Object& o)
{
    o.HasMember("a");
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

Requirement::Type Card::GetRecipeType() const
{
    return recipeType_;
}

bool Card::CanAssemble(std::set<std::shared_ptr<Card>>& parts)
{
    if (parts.size() != requirements_.size()) {
        return false;
    }
    for (const auto& r : requirements_) {
        auto found = std::find_if(parts.begin(), parts.end(), [&](const auto& p) { return r.Matches(p.get()); });
        if (found == parts.end()) {
            return false;
        }
        parts.erase(found);
    }
    return true;
}
