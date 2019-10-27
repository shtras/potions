#include <assert.h>
#include <sstream>
#include <algorithm>
#include <random>

#include "Game.h"

#include "Utils/Utils.h"
namespace Engine
{
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
    if (!parseCards(ss.str())) {
        cards_.clear();
        return false;
    }
    return true;
}

bool Game::parseCards(std::string filename)
{
    auto cont = Utils::ReadFile(filename);
    rapidjson::Document d;
    d.Parse(cont);
    if (d.HasParseError()) {
        return false;
    }
    auto cardsO = Utils::GetT<rapidjson::Value::ConstObject>(d, "cards");
    if (!cardsO) {
        return false;
    }
    const auto& cards = *cardsO;
    for (auto itr = cards.MemberBegin(); itr != cards.MemberEnd(); ++itr) {
        if (!itr->value.IsObject()) {
            return false;
        }
        auto idxStr = itr->name.GetString();
        int idx = std::stoi(idxStr);
        if (cards_.count(idx) > 0) {
            return false;
        }
        cards_[idx] = std::make_unique<Card>();
        bool res = cards_[idx]->Parse(idx, itr->value);
        if (!res) {
            return false;
        }
    }
    if (cards_.size() != cards.MemberCount()) {
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
    if (p->HandSize() >= rules_->MaxHandToDraw) {
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

Card* Game::GetCard(int idx) const
{
    if (cards_.count(idx) == 0) {
        return nullptr;
    }
    return cards_.at(idx).get();
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

void Game::Prepare(int numPlayers)
{
    for (int i = 0; i < numPlayers; ++i) {
        players_.push_back(std::make_unique<Player>());
    }
    deck_.reserve(cards_.size());
    for (const auto& p : cards_) {
        deck_.push_back(p.second.get());
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck_.begin(), deck_.end(), g);
    for (size_t i = 0; i < rules_->InitialClosetSize; ++i) {
        auto card = getTopCard();
        closet_->AddCard(card);
    }
    for (size_t i = 0; i < rules_->InitialHandSize; ++i) {
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
} // namespace Engine
