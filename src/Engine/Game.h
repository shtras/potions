#pragma once

#include <map>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "World.h"
#include "Card.h"
#include "Player.h"
#include "Closet.h"
#include "Rules.h"
#include "Move.h"

namespace Engine
{
class Game
{
public:
    enum class TurnState { Drawing, Playing, Done };
    bool DrawCard();
    bool DiscardCard(Card* card);
    bool EndTurn();
    bool Assemble(Card* card, std::set<Card*> parts);
    bool Init(std::string filename);
    void Prepare(int numPlayers);
    bool ValidateMove(Move* move);
    void PerformMove(Move* move);
    bool FromJson(std::string& json);
    World* GetWorld() const;
    void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w) const;

private:
    Player* getActivePlayer();
    void advanceState();
    Card* getTopCard();
    std::string stateToString(TurnState state) const;

    std::unique_ptr<World> world_ = std::make_unique<World>();
    std::unique_ptr<Closet> closet_ = nullptr;
    std::vector<Card*> deck_ = {};
    std::vector<std::unique_ptr<Player>> players_ = {};
    size_t activePlayerIdx_ = 0;
    TurnState turnState_ = TurnState::Drawing;
};
} // namespace Engine
