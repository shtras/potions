#include <algorithm>

#include "spdlog_wrap.h"

#include "Utils/Utils.h"
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

bool Card::Parse(const rapidjson::Value& o)
{
    if (!o.HasMember("ingredient")) {
        return false;
    }
    if (o["ingredient"].IsInt()) {
        ingredients_.insert(o["ingredient"].GetInt());
    } else if (o["ingredient"].IsArray()) {
        const auto& ingredients = o["ingredient"].GetArray();
        for (rapidjson::SizeType i = 0; i < ingredients.Size(); ++i) {
            if (!ingredients[i].IsInt()) {
                return false;
            }
            ingredients_.insert(ingredients[i].GetInt());
        }
    } else {
        return false;
    }
    auto nameO = Utils::GetT<std::string>(o, "name");
    if (!nameO) {
        return false;
    }
    name_ = *nameO;
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
    if (type_ != Type::Recipe) {
        return false;
    }
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

void Card::Assemble(std::set<AssemblePart*>& parts)
{
    assert(assembledParts_.empty());
    assert(CanAssemble(parts));
    for (auto part : parts) {
        assembledParts_.insert(part->GetCard());
    }
    assembled_ = true;
}

void Card::Disassemble()
{
    assembled_ = false;
    assembledParts_.clear();
}
