#include <assert.h>
#include <sstream>

#include "Game.h"

#include "Utils/Utils.h"
namespace Engine
{
Game::Game(std::string&& name)
    : name_(name)
{
}

bool Game::Init(std::string filename)
{
    auto cont = Utils::ReadFile(filename);
    if (cont.empty()) {
        return false;
    }
    rapidjson::Document d;
    d.Parse(cont);
    if (d.HasParseError()) {
        return false;
    }
    auto resPrefixO = Utils::GetT<std::string>(d, "resPrefix");
    if (!resPrefixO) {
        return false;
    }
    auto cardsFileO = Utils::GetT<std::string>(d, "cardsFile");
    if (!cardsFileO) {
        return false;
    }
    std::stringstream ss;
    ss << *resPrefixO << *cardsFileO;
    if (!world_->ParseCards(ss.str())) {
        return false;
    }
    closet_ = std::make_unique<Closet>(world_.get());
    return true;
}

bool Game::FromJson(const std::string& json)
{
    rapidjson::Document d;
    d.Parse(json);
    if (d.HasParseError()) {
        return false;
    }
    auto nameO = Utils::GetT<std::string>(d, "name");
    if (!nameO) {
        return false;
    }
    name_ = *nameO;
    auto turnO = Utils::GetT<std::string>(d, "turn");
    if (!turnO) {
        return false;
    }
    std::string turn = *turnO;
    auto stateO = Utils::GetT<std::string>(d, "state");
    if (!stateO) {
        return false;
    }
    std::string stateStr = *stateO;
    if (stateStr == "drawing") {
        turnState_ = TurnState::Drawing;
    } else if (stateStr == "playing") {
        turnState_ = TurnState::Playing;
    } else if (stateStr == "done") {
        turnState_ = TurnState::Done;
    } else {
        return false;
    }
    auto closetO = Utils::GetT<rapidjson::Value::ConstObject>(d, "closet");
    if (!closetO) {
        return false;
    }
    closet_ = std::make_unique<Closet>(world_.get());
    bool res = closet_->FromJson(*closetO);
    if (!res) {
        return false;
    }
    return true;
}

Card* Game::getTopCard()
{
    auto card = deck_.back();
    deck_.pop_back();
    return card;
}

bool Game::DrawCard()
{
    if (turnState_ != TurnState::Drawing) {
        return false;
    }
    auto p = getActivePlayer();
    if (p->HandSize() >= world_->GetRules()->MaxHandToDraw) {
        return false;
    }
    auto card = getTopCard();
    p->AddCard(card);
    advanceState();
    return true;
}

bool Game::EndTurn()
{
    if (turnState_ != TurnState::Done) {
        return false;
    }
    ++activePlayerIdx_;
    if (activePlayerIdx_ >= players_.size()) {
        activePlayerIdx_ = 0;
    }
    advanceState();
    return true;
}

bool Game::DiscardCard(Card* card)
{
    if (turnState_ != TurnState::Playing) {
        return false;
    }
    auto p = getActivePlayer();
    if (!p->DiscardCard(card)) {
        return false;
    }
    closet_->AddCard(card);
    advanceState();
    return true;
}

Player* Game::getActivePlayer()
{
    return players_[activePlayerIdx_].get();
}

void Game::advanceState()
{
    switch (turnState_) {
        case TurnState::Drawing:
            turnState_ = TurnState::Playing;
            break;
        case TurnState::Playing:
            turnState_ = TurnState::Done;
            break;
        case TurnState::Done:
            turnState_ = TurnState::Drawing;
            break;
        default:
            assert(0);
    }
}

bool Game::Assemble(Card* card, std::set<Card*> parts)
{
    if (!card->CanAssemble(parts)) {
        return false;
    }
    auto p = getActivePlayer();
    if (!p->HasCard(card)) {
        return false;
    }
    card->Assemble(parts);
    return true;
}

void Game::Prepare()
{
    world_->PrepareDeck(deck_);
    for (size_t i = 0; i < world_->GetRules()->InitialClosetSize; ++i) {
        auto card = getTopCard();
        closet_->AddCard(card);
    }
    for (size_t i = 0; i < world_->GetRules()->InitialHandSize; ++i) {
        for (const auto& p : players_) {
            auto card = getTopCard();
            p->AddCard(card);
        }
    }
}

bool Game::ValidateMove(Move* move)
{
    switch (move->GetAction()) {
        case Move::Action::Draw:
            break;
        case Move::Action::Skip:
            break;
        case Move::Action::Assemble:
            break;
        case Move::Action::Cast:
            break;
        default:
            assert(0);
    }
    return true;
}

void Game::PerformMove(Move* move)
{
    switch (move->GetAction()) {
        case Move::Action::Draw:
            break;
        case Move::Action::Skip:
            break;
        case Move::Action::Assemble:
            break;
        case Move::Action::Cast:
            break;
        default:
            assert(0);
    }
}

void Game::ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, bool hideInactive) const
{
    w.StartObject();
    w.Key("name");
    w.String(name_);
    w.Key("state");
    w.String(stateToString(turnState_));
    w.Key("closet");
    closet_->ToJson(w);
    w.Key("players");
    w.StartArray();
    for (size_t i = 0; i < players_.size(); ++i) {
        const auto& player = players_[i];
        bool hidden = hideInactive && (activePlayerIdx_ != i);
        player->ToJson(w, hidden);
    }
    w.EndArray();
    w.Key("deck");
    if (hideInactive) {
        w.Uint64(deck_.size());
    } else {
        w.StartArray();
        for (auto card : deck_) {
            w.Int(card->GetID());
        }
        w.EndArray();
    }
    w.EndObject();
}

const std::string& Game::GetName() const
{
    return name_;
}

void Game::AddPlayer(std::string& user)
{
    auto player = std::make_unique<Player>(world_.get(), user);
    players_.push_back(std::move(player));
}

std::string Game::stateToString(TurnState state) const
{
    switch (state) {
        case TurnState::Drawing:
            return "drawing";
        case TurnState::Playing:
            return "playing";
        case TurnState::Done:
            return "done";
        default:
            assert(0);
    }
    return "Unknown";
}

World* Game::GetWorld() const
{
    return world_.get();
}
} // namespace Engine
