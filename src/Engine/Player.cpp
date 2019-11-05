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
    auto scoreO = Utils::GetT<int>(o, "score");
    if (!scoreO) {
        return false;
    }
    score_ = *scoreO;
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

void Player::ToJson(bsoncxx::builder::stream::document& d, bool hidden /* = false*/) const
{
    d << "user" << user_;
    d << "score" << score_;
    auto t = d << "hand";
    if (hidden) {
        t << bsoncxx::types::b_int64{(int64_t)hand_.size()};
    } else {
        bsoncxx::builder::stream::array a;
        for (auto card : hand_) {
            a << card->GetID();
        }
        t << a;
    }
    auto t1 = d << "table";
    bsoncxx::builder::stream::document d2;
    for (auto card : assembledCards_) {
        bsoncxx::builder::stream::array a;
        for (const auto& part : card->GetParts()) {
            a << part->GetID();
        }
        d2 << std::to_string(card->GetID()) << a;
    }
    t1 << d2;
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

void Player::AddScore(int score)
{
    score_ += score;
}
} // namespace Engine
