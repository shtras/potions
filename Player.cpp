#include "Player.h"

void Player::AddCard(std::shared_ptr<Card> card)
{
    hand_.insert(card);
}

size_t Player::HandSize() const
{
    return hand_.size();
}

bool Player::DiscardCard(std::shared_ptr<Card> card)
{
    if (!removeFromHand(card)) {
        return false;
    }
    return true;
}

bool Player::removeFromHand(std::shared_ptr<Card> card)
{
    if (hand_.count(card) == 0) {
        return false;
    }
    hand_.erase(card);
    return true;
}
