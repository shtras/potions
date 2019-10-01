#pragma once

#include <map>
#include <list>
#include <vector>
#include <memory>

#include "Card.h"
#include "Player.h"
#include "Closet.h"

class Game
{
public:
    enum class TurnState { Drawing, Playing, Done };
    bool DrawCard();
    bool DiscardCard(std::shared_ptr<Card> card);
    bool EndTurn();
    bool Assemble(std::shared_ptr<Card> card, std::list<std::shared_ptr<Card>> parts);

private:
    std::shared_ptr<Player>& getActivePlayer();
    void advanceState();

    std::unique_ptr<Closet> closet_ = std::make_unique<Closet>();
    std::list<std::shared_ptr<Card>> deck_;
    std::vector<std::shared_ptr<Player>> players_;
    size_t activePlayerIdx_ = 0;
    TurnState turnState_ = TurnState::Drawing;
};
