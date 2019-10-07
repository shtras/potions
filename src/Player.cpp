#include "Player.h"

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
