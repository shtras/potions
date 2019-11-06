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

bool Player::FromJson(const bsoncxx::document::view& bson)
{
    const auto& hand = bson["hand"];
    if (hand.type() != bsoncxx::type::k_array) {
        return false;
    }
    for (const auto& elm : hand.get_array().value) {
        if (elm.type() != bsoncxx::type::k_int32) {
            return false;
        }
        auto idx = elm.get_int32().value;
        auto card = world_->GetCard(idx);
        hand_.insert(card);
    }
    const auto& score = bson["score"];
    if (score.type() != bsoncxx::type::k_int32) {
        return false;
    }
    score_ = score.get_int32().value;
    const auto& table = bson["table"];
    if (table.type() != bsoncxx::type::k_document) {
        return false;
    }
    for (const auto& elm : table.get_document().value) {
        auto cardIdx = std::stoi(std::string(elm.key()));
        auto card = world_->GetCard(cardIdx);
        if (!card) {
            return false;
        }
        if (elm.type() != bsoncxx::type::k_array) {
            return false;
        }
        std::vector<Card*> partsVec;
        for (const auto& part : elm.get_array().value) {
            if (part.type() != bsoncxx::type::k_int32) {
                return false;
            }
            partsVec.push_back(world_->GetCard(part.get_int32().value));
        }
        card->Assemble(partsVec);
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
        t << bsoncxx::types::b_int64{static_cast<int64_t>(hand_.size())};
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
