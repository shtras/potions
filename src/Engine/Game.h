#pragma once

#include <map>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <memory>

#include "bsoncxx_wrap.h"

#include "Utils/Utils.h"
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
    enum class TurnState { Preparing, Drawing, Playing, Done };

    Game(std::string&& name);

    bool Init(std::string filename);
    bool ValidateMove(const Move* move) const;
    void PerformMove(std::shared_ptr<Move> move);
    bool Parse(const std::string& str);
    bool FromJson(const bsoncxx::document::view& bson);
    World* GetWorld() const;
    void ToJson(bsoncxx::builder::stream::document& d, std::string_view forUser = "") const;
    bool AddPlayer(std::string& user);
    const std::string& GetName() const;
    void Start();
    int64_t LastUpdated() const;
    const std::list<std::shared_ptr<Move>> GetMoves() const;

private:
    void drawCard();
    void discardCard(Card* card);
    void endTurn();
    void performCast(const Move& move);
    void performCastTransform(const Move& move);
    void performCastReveal(const Move& move);
    void performCastDestroy(const Move& move);
    bool validateCast(const Move& move) const;
    bool validateCastTransform(const Move& move) const;
    bool validateCastReveal(const Move& move) const;
    bool validateCastDestroy(const Move& move) const;
    void assemble(Card* card, std::vector<Card*> parts);
    Player* getActivePlayer() const;
    void advanceState();
    Card* getTopCard();
    std::string stateToString(TurnState state) const;

    std::unique_ptr<World> world_ = std::make_unique<World>();
    std::unique_ptr<Closet> closet_ = nullptr;
    std::vector<Card*> deck_ = {};
    std::vector<std::unique_ptr<Player>> players_ = {};
    size_t activePlayerIdx_ = 0;
    TurnState turnState_ = TurnState::Preparing;
    std::string name_;
    std::list<std::shared_ptr<Move>> moves_;
    int64_t lastMove_ = Utils::GetTime();
};
} // namespace Engine
