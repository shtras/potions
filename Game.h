#pragma once

#include <map>
#include <list>
#include <set>
#include <map>
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
    bool DiscardCard(Card* card);
    bool EndTurn();
    bool Assemble(Card* card, std::set<AssemblePart*> parts);
    Card* GetCard(int idx);
    bool Init(rapidjson::Document& d);

private:
    std::shared_ptr<Player>& getActivePlayer();
    void advanceState();

    std::unique_ptr<Closet> closet_ = std::make_unique<Closet>();
    std::map<int, std::unique_ptr<Card>> cards_ = {};
    std::list<Card*> deck_ = {};
    std::vector<std::shared_ptr<Player>> players_ = {};
    size_t activePlayerIdx_ = 0;
    TurnState turnState_ = TurnState::Drawing;
};
