#include <assert.h>

#include "Game.h"

#include "Utils/Utils.h"

bool Game::Init(std::string filename)
{
    auto cont = Utils::ReadFile(filename);
    rapidjson::Document d;
    d.Parse(cont);
    return true;
}

bool Game::DrawCard()
{
    if (turnState_ != TurnState::Drawing) {
        return false;
    }
    auto& p = getActivePlayer();
    if (p->HandSize() >= 7) {
        return false;
    }
    auto card = deck_.front();
    deck_.pop_front();
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
    auto& p = getActivePlayer();
    if (!p->DiscardCard(card)) {
        return false;
    }
    closet_->AddCard(card);
    advanceState();
    return true;
}

std::shared_ptr<Player>& Game::getActivePlayer()
{
    return players_[activePlayerIdx_];
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

bool Game::Assemble(Card* card, std::set<AssemblePart*> parts)
{
    if (!card->CanAssemble(parts)) {
        return false;
    }
    auto& p = getActivePlayer();
    if (!p->HasCard(card)) {
        return false;
    }

    return true;
}