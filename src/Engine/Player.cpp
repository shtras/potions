#include "Player.h"

namespace Engine
{
Player::Player(World* w)
    : world_(w)
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

void Player::ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w) const
{
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
