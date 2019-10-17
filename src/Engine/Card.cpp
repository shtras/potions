#include <algorithm>

#include "spdlog_wrap.h"

#include "Utils/Utils.h"
#include "Card.h"

namespace Engine
{
bool Requirement::Matches(const Card* card) const
{
    switch (type_) {
        case Type::Ingredient:
            return !card->IsAssembled() && ids_.count(card->GetIngredient()) > 0;
        case Type::Recipe:
            return card->IsAssembled() && ids_.count(card->GetID()) > 0;
        default:
            assert(0);
    }
    return false;
}

bool Requirement::Parse(const rapidjson::Value& o)
{
    auto typeO = Utils::GetT<std::string>(o, "type");
    if (!typeO) {
        return false;
    }
    if (*typeO == "ingredient") {
        type_ = Type::Ingredient;
    } else if (*typeO == "recipe") {
        type_ = Type::Recipe;
    } else {
        return false;
    }
    const auto idsO = Utils::GetT<rapidjson::Value::ConstArray>(o, "ids");
    if (!idsO) {
        return false;
    }
    const auto& ids = *idsO;
    for (rapidjson::SizeType i = 0; i < ids.Size(); ++i) {
        if (!ids[i].IsInt()) {
            return false;
        }
        int id = ids[i].GetInt();
        ids_.insert(id);
    }
    return true;
}

bool Card::Parse(int id, const rapidjson::Value& o)
{
    id_ = id;
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
    auto typeO = Utils::GetT<std::string>(o, "type");
    if (!typeO) {
        return false;
    }
    if (*typeO == "recipe") {
        type_ = Type::Recipe;
    } else if (*typeO == "spell") {
        type_ = Type::Spell;
    } else {
        return false;
    }

    if (type_ == Type::Recipe) {
        auto requirementsO = Utils::GetT<rapidjson::Value::ConstArray>(o, "requirements");
        if (!requirementsO) {
            return false;
        }
        for (rapidjson::SizeType i = 0; i < (*requirementsO).Size(); ++i) {
            const auto& req = (*requirementsO)[i];
            requirements_.emplace_back();
            bool res = requirements_.back().Parse(req);
            if (!res) {
                return false;
            }
        }
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

int Card::GetScore() const
{
    return score_;
}

int Card::GetID() const
{
    return id_;
}

bool Card::CanAssemble(const std::set<Card*>& parts) const
{
    if (type_ != Type::Recipe) {
        return false;
    }
    if (parts.size() != requirements_.size()) {
        return false;
    }
    for (const auto& r : requirements_) {
        auto found = std::find_if(parts.begin(), parts.end(), [&](const auto& p) { return r.Matches(p); });
        if (found == parts.end()) {
            return false;
        }
    }
    return true;
}

bool Card::IsAssembled() const
{
    return assembled_;
}

void Card::Assemble(std::set<Card*>& parts)
{
    assert(assembledParts_.empty());
    assert(CanAssemble(parts));
    for (auto part : parts) {
        assembledParts_.insert(part);
    }
    assembled_ = true;
}

void Card::Disassemble()
{
    assembled_ = false;
    assembledParts_.clear();
}
} // namespace Engine
