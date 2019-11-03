#include "Utils/Utils.h"

#include "Player.h"

namespace Engine
{
Player::Player(World* w, std::string& user)
    : world_(w)
    , user_(user)
{
}

void Player::AddCard(Card* card)
{
    hand_.insert(card);
}

size_t Player::HandSize() const
{
    return hand_.size();
}

void Player::DiscardCard(Card* card)
{
    assert(HasCard(card));
    removeFromHand(card);
}

bool Player::FromJson(const rapidjson::Value::ConstObject& o)
{
    auto handO = Utils::GetT<rapidjson::Value::ConstArray>(o, "hand");
    if (!handO) {
        return false;
    }
    const auto& hand = *handO;
    for (rapidjson::SizeType i = 0; i < hand.Size(); ++i) {
        auto idxO = Utils::GetT<int>(hand[i]);
        if (!idxO) {
            return false;
        }
        auto card = world_->GetCard(*idxO);
        hand_.insert(card);
    }
    auto tableO = Utils::GetT<rapidjson::Value::ConstObject>(o, "table");
    if (!tableO) {
        return false;
    }
    const auto& table = *tableO;
    for (auto itr = table.MemberBegin(); itr != table.MemberEnd(); ++itr) {
        auto cardIdx = std::stoi(itr->name.GetString());
        auto card = world_->GetCard(cardIdx);
        if (!card) {
            return false;
        }
        auto partsO = Utils::GetT<rapidjson::Value::ConstArray>(itr->value);
        if (!partsO) {
            return false;
        }
        const auto& parts = *partsO;
        std::vector<Card*> partsSet;
        for (rapidjson::SizeType i = 0; i < parts.Size(); ++i) {
            auto partO = Utils::GetT<int>(parts[i]);
            if (!partO) {
                return false;
            }
            partsSet.push_back(world_->GetCard(*partO));
        }
        card->Assemble(partsSet);
        assembledCards_.insert(card);
    }
    return true;
}

Card* Player::FindAssembledWithPart(Card* part) const
{
    for (auto card : assembledCards_) {
        if (card->HasPart(part)) {
            return card;
        }
    }
    return nullptr;
}

void Player::ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, bool hidden /* = false*/) const
{
    w.StartObject();
    w.Key("user");
    w.String(user_);
    w.Key("hand");
    if (hidden) {
        w.Uint64(hand_.size());
    } else {
        w.StartArray();
        for (auto card : hand_) {
            w.Int(card->GetID());
        }
        w.EndArray();
    }
    w.Key("table");
    w.StartObject();
    for (auto card : assembledCards_) {
        w.Key(std::to_string(card->GetID()));
        w.StartArray();
        for (const auto& part : card->GetParts()) {
            w.Int(part->GetID());
        }
        w.EndArray();
    }
    w.EndObject();
    w.EndObject();
}

bool Player::removeFromHand(Card* card)
{
    if (hand_.count(card) == 0) {
        return false;
    }
    hand_.erase(card);
    return true;
}

bool Player::HasCard(Card* card) const
{
    return hand_.count(card) > 0;
}

void Player::AddAssembled(Card* card)
{
    assembledCards_.insert(card);
}

void Player::RemoveAssembled(Card* card)
{
    assert(assembledCards_.count(card) > 0);
    assembledCards_.erase(card);
}

const std::string& Player::GetUser() const
{
    return user_;
}

bool Player::HasAssembled(Card* card) const
{
    return assembledCards_.count(card) > 0;
}
} // namespace Engine
