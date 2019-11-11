#include <algorithm>

#include "spdlog_wrap.h"

#include "Utils/Utils.h"
#include "Card.h"

namespace Engine
{
bool Requirement::Matches(const Card* card) const
{
    if (!card) {
        return false;
    }
    switch (type_) {
        case Type::Ingredient:
            if (card->IsAssembled()) {
                return false;
            }
            for (auto id : ids_) {
                if (card->HasIngredient(id)) {
                    return true;
                }
            }
            return false;
        case Type::Recipe:
            return card->IsAssembled() && ids_.count(card->GetID()) > 0;
        default:
            assert(0);
    }
    return false;
}

bool Requirement::Parse(const bsoncxx::document::view& d)
{
    auto type = d["type"];
    if (!type || type.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string typeStr(type.get_utf8().value);
    if (typeStr == "ingredient") {
        type_ = Type::Ingredient;
    } else if (typeStr == "recipe") {
        type_ = Type::Recipe;
    } else {
        return false;
    }
    auto ids = d["ids"];
    if (!ids || ids.type() != bsoncxx::type::k_array) {
        return false;
    }
    for (const auto& elm : ids.get_array().value) {
        auto type1 = elm.type();
        if (elm.type() != bsoncxx::type::k_int32) {
            return false;
        }
        ids_.insert(elm.get_int32().value);
    }

    return true;
}

bool Card::Parse(int id, const bsoncxx::document::view& d)
{
    id_ = id;
    auto ing = d["ingredient"];
    if (!ing) {
        return false;
    }
    if (ing.type() == bsoncxx::type::k_int32) {
        ingredients_.insert(ing.get_int32().value);
    } else if (ing.type() == bsoncxx::type::k_array) {
        for (auto ingItr : ing.get_array().value) {
            if (ingItr.type() != bsoncxx::type::k_int32) {
                return false;
            }
            ingredients_.insert(ingItr.get_int32().value);
        }
    } else {
        return false;
    }
    auto name = d["name"];
    if (!name || name.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    name_ = std::string(name.get_utf8().value);
    auto score = d["score"];
    if (!score || score.type() != bsoncxx::type::k_int32) {
        return false;
    }
    score_ = score.get_int32().value;
    auto type = d["type"];
    if (!type || type.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string typeStr(type.get_utf8().value);
    if (typeStr == "recipe") {
        type_ = Type::Recipe;
    } else if (typeStr == "spell") {
        type_ = Type::Spell;
    } else {
        return false;
    }

    if (type_ == Type::Recipe) {
        auto requirements = d["requirements"];
        if (!requirements || requirements.type() != bsoncxx::type::k_array) {
            return false;
        }
        for (const auto& req : requirements.get_array().value) {
            if (req.type() != bsoncxx::type::k_document) {
                return false;
            }
            requirements_.emplace_back();
            bool res = requirements_.back().Parse(req.get_document().view());
            if (!res) {
                return false;
            }
        }
    }

    return true;
} // namespace Engine

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

bool Card::HasIngredient(int id) const
{
    return ingredients_.count(id) > 0;
}

int Card::GetScore() const
{
    return score_;
}

int Card::GetID() const
{
    return id_;
}

bool Card::CanAssemble(const std::vector<Card*>& parts) const
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

bool Card::HasPart(Card* c) const
{
    return assembledParts_.count(c) > 0;
}

bool Card::IsAssembled() const
{
    return assembled_;
}

void Card::Assemble(std::vector<Card*>& parts)
{
    assert(assembledParts_.empty());
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

Card::Type Card::GetType() const
{
    return type_;
}

const std::set<Card*>& Card::GetParts() const
{
    return assembledParts_;
}
} // namespace Engine
