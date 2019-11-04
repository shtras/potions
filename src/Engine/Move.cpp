#include "Utils/Utils.h"

#include "Move.h"

namespace Engine
{
Move::Move(std::string& user)
    : user_(user)
{
}

void Move::ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w) const
{
    w.StartObject();
    w.Key("user");
    w.String(user_);
    w.Key("action");
    switch (action_) {
        case Engine::Move::Action::Draw:
            w.String("draw");
            break;
        case Engine::Move::Action::Skip:
            w.String("skip");
            break;
        case Engine::Move::Action::Assemble:
            w.String("assemble");
            break;
        case Engine::Move::Action::Discard:
            w.String("discard");
            break;
        case Engine::Move::Action::Cast:
            w.String("cast");
            break;
        case Engine::Move::Action::EndTurn:
            w.String("endturn");
            break;
        default:
            w.String("unknown");
            break;
    }
    w.Key("card");
    w.Int(card_);
    w.Key("parts");
    w.StartArray();
    for (const auto& part : parts_) {
        w.StartObject();
        w.Key("id");
        w.Int(part.id);
        w.Key("type");
        switch (part.type) {
            case Requirement::Type::Ingredient:
                w.String("ingredient");
                break;
            case Requirement::Type::Recipe:
                w.String("recipe");
                break;
            case Requirement::Type::None:
                w.String("none");
                break;
            default:
                w.String("unknown");
                break;
        }
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
}

bool Move::FromJson(const rapidjson::Value::ConstObject& o)
{
    auto actionO = Utils::GetT<std::string>(o, "action");
    if (!actionO) {
        return false;
    }
    auto cardO = Utils::GetT<int>(o, "card");
    if (cardO) {
        card_ = *cardO;
    }
    auto action = *actionO;
    if (action == "draw") {
        action_ = Action::Draw;
    } else if (action == "skip") {
        action_ = Action::Skip;
    } else if (action == "discard") {
        action_ = Action::Discard;
        if (card_ == -1) {
            return false;
        }
    } else if (action == "assemble") {
        action_ = Action::Assemble;
        if (card_ == -1) {
            return false;
        }
    } else if (action == "cast") {
        action_ = Action::Cast;
        if (card_ == -1) {
            return false;
        }
    } else if (action == "endturn") {
        action_ = Action::EndTurn;
    } else {
        return false;
    }

    if (action_ == Action::Assemble || action_ == Action::Cast) {
        auto partsO = Utils::GetT<rapidjson::Value::ConstArray>(o, "parts");
        if (!partsO) {
            return false;
        }
        const auto& parts = *partsO;
        for (rapidjson::SizeType i = 0; i < parts.Size(); ++i) {
            const auto& part = parts[i];
            auto typeO = Utils::GetT<std::string>(part, "type");
            if (!typeO) {
                return false;
            }
            auto type = Requirement::Type::None;
            if (*typeO == "ingredient") {
                type = Requirement::Type::Ingredient;
            } else if (*typeO == "recipe") {
                type = Requirement::Type::Recipe;
            } else {
                return false;
            }
            auto idO = Utils::GetT<int>(part, "id");
            if (!idO) {
                return false;
            }
            parts_.emplace_back();
            parts_.back().id = *idO;
            parts_.back().type = type;
        }
    }
    return true;
}

bool Move::Parse(std::string moveJson)
{
    rapidjson::Document d;
    d.Parse(moveJson);
    if (d.HasParseError()) {
        return false;
    }
    const auto& o = d.Get<rapidjson::Value::ConstObject>();
    return FromJson(o);
}

Move::Action Move::GetAction() const
{
    return action_;
}

const std::string& Move::GetUser() const
{
    return user_;
}

int Move::GetCard() const
{
    return card_;
}

std::vector<Card*> Move::GetParts(World* world) const
{
    std::vector<Card*> res;
    res.reserve(parts_.size());
    for (auto part : parts_) {
        res.push_back(world->GetCard(part.id));
    }
    return res;
}

} // namespace Engine
