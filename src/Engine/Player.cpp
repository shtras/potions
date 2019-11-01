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

bool Player::DiscardCard(Card* card)
{
    if (!removeFromHand(card)) {
        return false;
    }
    return true;
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

bool Player::HasCard(Card* card)
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
} // namespace Engine
