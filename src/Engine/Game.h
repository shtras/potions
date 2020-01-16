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
    enum class TurnState { Preparing, Drawing, DrawPlaying, Playing, Done };
    explicit Game(std::string&& name);

    bool Init(std::string filename);
    bool ValidateMove(const Move* move) const;
    void PerformMove(std::shared_ptr<Move>& move);
    bool Parse(const std::string& str);
    bool FromJson(const bsoncxx::document::view& bson);
    World* GetWorld() const;
    void ToJson(bsoncxx::builder::stream::document& d, std::string_view forUser = "") const;
    bool AddPlayer(std::string& user);
    const std::string& GetName() const;
    void Start();
    int64_t LastUpdated() const;
    const std::list<std::shared_ptr<Move>> GetMoves() const;
    void ActivateExpansion(World::DeckType type);
    int GetExpansions() const;

private:
    struct SpecialState
    {
        enum class StateType { None, DrawExtra, Disassembling, Giving, Transfiguring };
        void ToJson(
            bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d)
            const;
        bool FromJson(const bsoncxx::document::view& bson);

        StateType State = StateType::None;
        size_t PlayerIdx = 0;
        int DrawRemains = 0;
        int IngredientRequested = -1;
        const std::map<StateType, std::string_view> StateNames = {{StateType::None, "none"},
            {StateType::DrawExtra, "drawextra"}, {StateType::Disassembling, "disassembling"},
            {StateType::Giving, "giving"}, {StateType::Transfiguring, "transfiguring"}};
    };
    void drawCard(World::DeckType type);
    void discardCard(Card* card);
    void endTurn();
    void performCast(const Move& move);
    void performCastTransform(const Move& move);
    void performCastReveal(const Move& move);
    void performCastDestroy(const Move& move);
    void performCastWhirpool(const Move& move);
    void performCastNecessity(const Move& move);
    void performCastMagicReveal(const Move& move);
    void performCastCreate(const Move& move);
    void performCastTransfigure(const Move& move);
    void performCastOverthrow(const Move& move);
    void performCastDiversity(const Move& move);
    void performCastForest(const Move& move);
    void performCastConcience(const Move& move);
    void performCastSpeedup(const Move& move);
    void performDisassemble(const Move& move);
    void performSpecialDiscard(const Move& move);
    void performSpecialAssemble(const Move& move);
    void performSpecialMove(std::shared_ptr<Move>& move);
    bool validateCast(const Move& move) const;
    bool validateCastTransform(const Move& move) const;
    bool validateCastReveal(const Move& move) const;
    bool validateCastDestroy(const Move& move) const;
    bool validateCastGlobalReveal(const Move& move) const;
    bool validateCastCreate(const Move& move) const;
    bool validateCastMagicReveal(const Move& move) const;
    bool validateCastOverthrow(const Move& move) const;
    bool validateCastForest(const Move& move) const;
    bool validateCastConcience(const Move& move) const;
    bool validateDraw(const Move& move) const;
    bool validateSkip() const;
    bool validateDiscard(const Move& move) const;
    bool validateSpecialDiscard(const Move& move) const;
    bool validateSpecialAssembly(const Move& move) const;
    bool validateAssemble(const Move& move) const;
    bool validateDisassemble(const Move& move) const;
    bool validateSpecialMove(const Move& move) const;
    bool validateSpecialEndTurn(const Move& move) const;
    void assemble(Card* card, std::vector<Card*> parts);
    void checkGrowthTalisman();
    void checkIncomeTalisman();
    Player* getActivePlayer() const;
    Player* findPlayer(const std::string& user) const;
    void advanceState();
    void advanceSpecialState();
    Card* getTopCard(World::DeckType type);
    std::string stateToString(TurnState state) const;
    bool hasExpansion(World::DeckType type);

    SpecialState specialState_;
    std::unique_ptr<World> world_ = std::make_unique<World>();
    std::unique_ptr<Closet> closet_ = nullptr;
    std::map<World::DeckType, std::vector<Card*>> decks_ = {{World::DeckType::Base, {}}};
    std::vector<std::unique_ptr<Player>> players_ = {};
    size_t activePlayerIdx_ = 0;
    TurnState turnState_ = TurnState::Preparing;
    std::string name_;
    std::list<std::shared_ptr<Move>> moves_;
    int64_t lastMove_ = Utils::GetTime();
    int expansions_ = 0;
    int extraPlayMoves_ = 0;

    const std::map<TurnState, std::string_view> turnStateNames_ = {
        {TurnState::Preparing, "preparing"}, {TurnState::Drawing, "drawing"},
        {TurnState::DrawPlaying, "drawplaying"}, {TurnState::Playing, "playing"},
        {TurnState::Done, "done"}};
};
} // namespace Engine
