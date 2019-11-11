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
    auto maybeD = Utils::ParseBson(cont);
    if (!maybeD) {
        return false;
    }
    auto d = (*maybeD).view();
    auto resPrefix = d["resPrefix"];
    if (!resPrefix || resPrefix.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    auto cardsFile = d["cardsFile"];
    if (!cardsFile || cardsFile.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::stringstream ss;
    ss << resPrefix.get_utf8().value << cardsFile.get_utf8().value;
    if (!world_->ParseCards(ss.str())) {
        return false;
    }
    closet_ = std::make_unique<Closet>(world_.get());
    return true;
}

Card* Game::getTopCard()
{
    auto card = deck_.back();
    deck_.pop_back();
    return card;
}

void Game::drawCard()
{
    assert(turnState_ == TurnState::Drawing);
    auto p = getActivePlayer();
    assert(p->HandSize() < world_->GetRules()->MaxHandToDraw);
    auto card = getTopCard();
    p->AddCard(card);
    if (p->HandSize() >= world_->GetRules()->MinCardsInHand) {
        advanceState();
    }
}

void Game::endTurn()
{
    assert(turnState_ == TurnState::Done);
    ++activePlayerIdx_;
    if (activePlayerIdx_ >= players_.size()) {
        activePlayerIdx_ = 0;
    }
    advanceState();
}

void Game::discardCard(Card* card)
{
    assert(turnState_ == TurnState::Playing);
    auto p = getActivePlayer();
    if (!closet_->HasIngredient(card->GetIngredient())) {
        p->AddScore(1);
    }
    p->DiscardCard(card);
    closet_->AddCard(card);
    advanceState();
}

Player* Game::getActivePlayer() const
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

void Game::assemble(Card* card, std::vector<Card*> parts)
{
    assert(card->CanAssemble(parts));
    auto p = getActivePlayer();
    assert(p->HasCard(card));
    std::set<Player*> playersToScore;
    playersToScore.insert(p);
    for (auto part : parts) {
        if (part->IsAssembled()) {
            for (auto& player : players_) {
                if (player->HasAssembled(part)) {
                    player->RemoveAssembled(part);
                    playersToScore.insert(player.get());
                    break;
                }
            }
            for (auto assembledPart : part->GetParts()) {
                closet_->AddCard(assembledPart);
            }
            part->Disassemble();
        } else {
            closet_->RemoveCard(part);
        }
    }
    for (auto player : playersToScore) {
        if (player == p) {
            player->AddScore(card->GetScore());
        } else {
            player->AddScore(card->GetScore() / 2);
        }
    }
    card->Assemble(parts);
    p->DiscardCard(card);
    p->AddAssembled(card);
    advanceState();
}

void Game::Start()
{
    if (players_.size() < world_->GetRules()->MinPlayers) {
        return;
    }
    if (turnState_ != TurnState::Preparing) {
        return;
    }
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
    turnState_ = TurnState::Drawing;
}

bool Game::ValidateMove(const Move* move) const
{
    auto activePlayer = getActivePlayer();
    if (activePlayer->GetUser() != move->GetUser()) {
        return false;
    }
    switch (move->GetAction()) {
        case Move::Action::Draw:
            return turnState_ == TurnState::Drawing && activePlayer->HandSize() < world_->GetRules()->MaxHandToDraw &&
                   !deck_.empty();
        case Move::Action::Skip:
            if (turnState_ == TurnState::Drawing &&
                (activePlayer->HandSize() >= world_->GetRules()->MaxHandToDraw || deck_.empty())) {
                return true;
            }
            if (turnState_ == TurnState::Playing && activePlayer->HandSize() == 0) {
                return true;
            }
            return false;
        case Move::Action::Discard:
            return turnState_ == TurnState::Playing && activePlayer->HasCard(world_->GetCard(move->GetCard()));
        case Move::Action::Assemble:
            return turnState_ == TurnState::Playing &&
                   world_->GetCard(move->GetCard())->CanAssemble(move->GetParts(world_.get()));
        case Move::Action::Cast:
            return validateCast(*move);
        case Move::Action::EndTurn:
            return turnState_ == TurnState::Done;
        default:
            assert(0);
    }
    return false;
}

void Game::PerformMove(std::shared_ptr<Move> move)
{
    assert(ValidateMove(move.get()));
    switch (move->GetAction()) {
        case Move::Action::Draw:
            drawCard();
            break;
        case Move::Action::Skip:
            advanceState();
            break;
        case Move::Action::Discard:
            discardCard(world_->GetCard(move->GetCard()));
            break;
        case Move::Action::Assemble:
            assemble(world_->GetCard(move->GetCard()), move->GetParts(world_.get()));
            break;
        case Move::Action::Cast:
            performCast(*move);
            break;
        case Move::Action::EndTurn:
            endTurn();
            break;
        default:
            assert(0);
    }
    lastMove_ = Utils::GetTime();
    moves_.push_back(move);
}

void Game::performCastTransform(const Move& move)
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    auto card = world_->GetCard(move.GetCard());
    Card* cardFromCloset = nullptr;
    Card* cardAssembled = nullptr;
    if (player->HasAssembled(parts[0])) {
        cardFromCloset = parts[1];
        cardAssembled = parts[0];
    } else {
        cardFromCloset = parts[0];
        cardAssembled = parts[1];
    }
    closet_->RemoveCard(cardFromCloset);
    closet_->AddCard(card);
    std::vector<Card*> v;
    cardFromCloset->Assemble(v);
    player->AddAssembled(cardFromCloset);
    for (auto part : cardAssembled->GetParts()) {
        closet_->AddCard(part);
    }
    cardAssembled->Disassemble();
    player->RemoveAssembled(cardAssembled);
    closet_->AddCard(cardAssembled);
    player->DiscardCard(card);
}

void Game::performCastReveal(const Move& move)
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    auto card = world_->GetCard(move.GetCard());
    auto newCard = parts[0];
    closet_->RemoveCard(newCard);
    player->DiscardCard(card);
    closet_->AddCard(card);
    player->AddCard(newCard);
}

void Game::performCastDestroy(const Move& move)
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    auto card = world_->GetCard(move.GetCard());
    player->DiscardCard(card);
    closet_->AddCard(card);
    auto cardToKeep = parts[0];
    auto cardToDestroy = player->FindAssembledWithPart(cardToKeep);
    for (auto part : cardToDestroy->GetParts()) {
        if (part == cardToKeep) {
            std::vector<Card*> v;
            part->Assemble(v);
            player->AddAssembled(part);
        } else {
            closet_->AddCard(part);
        }
    }
    cardToDestroy->Disassemble();
    player->RemoveAssembled(cardToDestroy);
    closet_->AddCard(cardToDestroy);
}

void Game::performCast(const Move& move)
{
    switch (move.GetCard()) {
        case 71:
        case 72:
            performCastTransform(move);
            break;
        case 73:
        case 74:
            performCastReveal(move);
            break;
        case 75:
        case 76:
            performCastDestroy(move);
            break;
    }
}

bool Game::validateCastTransform(const Move& move) const
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 2) {
        return false;
    }
    if (closet_->CanRemoveCard(parts[0]) && player->HasAssembled(parts[1])) {
        if (parts[0]->GetType() != Card::Type::Recipe) {
            return false;
        }
        return true;
    }
    if (closet_->CanRemoveCard(parts[1]) && player->HasAssembled(parts[0])) {
        if (parts[1]->GetType() != Card::Type::Recipe) {
            return false;
        }
        return true;
    }
    return false;
}

bool Game::validateCastReveal(const Move& move) const
{
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    if (!closet_->CanRemoveCard(parts[0])) {
        return false;
    }
    if (parts[0]->GetType() != Card::Type::Recipe) {
        return false;
    }
    return true;
}

bool Game::validateCastDestroy(const Move& move) const
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    if (parts[0]->GetType() != Card::Type::Recipe) {
        return false;
    }
    auto card = player->FindAssembledWithPart(parts[0]);
    if (!card) {
        return false;
    }
    return true;
}

bool Game::validateCast(const Move& move) const
{
    if (turnState_ != TurnState::Playing) {
        return false;
    }
    auto card = world_->GetCard(move.GetCard());
    if (!card || card->GetType() != Card::Type::Spell) {
        return false;
    }
    auto player = getActivePlayer();
    if (!player->HasCard(card)) {
        return false;
    }
    switch (move.GetCard()) {
        case 71:
        case 72:
            return validateCastTransform(move);
        case 73:
        case 74:
            return validateCastReveal(move);
        case 75:
        case 76:
            return validateCastDestroy(move);
    }
    return false;
}

bool Game::Parse(const std::string& str)
{
    return FromJson(bsoncxx::from_json(str));
}

bool Game::FromJson(const bsoncxx::document::view& bson)
{
    const auto& stateElm = bson["state"];
    if (!stateElm || stateElm.type() != bsoncxx::type::k_document) {
        return false;
    }
    const auto& state = stateElm.get_document().view();
    auto name = state["name"];
    if (!name || name.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    name_ = std::string(name.get_utf8().value);
    const auto& turn = state["turn"];
    if (!turn || turn.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string turnStr = std::string(turn.get_utf8().value);
    auto gameState = state["state"];
    if (!gameState || gameState.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string stateStr(gameState.get_utf8().value);
    if (stateStr == "preparing") {
        turnState_ = TurnState::Preparing;
    } else if (stateStr == "drawing") {
        turnState_ = TurnState::Drawing;
    } else if (stateStr == "playing") {
        turnState_ = TurnState::Playing;
    } else if (stateStr == "done") {
        turnState_ = TurnState::Done;
    } else {
        return false;
    }
    const auto& closet = state["closet"];
    if (!closet || closet.type() != bsoncxx::type::k_document) {
        return false;
    }
    closet_ = std::make_unique<Closet>(world_.get());
    bool res = closet_->FromJson(closet.get_document().view());
    if (!res) {
        return false;
    }
    const auto& players = state["players"];
    if (!players || players.type() != bsoncxx::type::k_array) {
        return false;
    }
    size_t i = 0;
    for (const auto& playerElm : players.get_array().value) {
        if (playerElm.type() != bsoncxx::type::k_document) {
            return false;
        }
        const auto& player = playerElm.get_document().view();
        const auto& user = player["user"];
        if (!user || user.type() != bsoncxx::type::k_utf8) {
            return false;
        }
        std::string userStr(user.get_utf8().value);
        if (userStr == turnStr) {
            activePlayerIdx_ = i;
        }
        auto p = std::make_unique<Player>(world_.get(), userStr);
        if (!p->FromJson(player)) {
            return false;
        }
        players_.push_back(std::move(p));
        ++i;
    }
    const auto& deck = state["deck"];
    if (!deck || deck.type() != bsoncxx::type::k_array) {
        return false;
    }
    for (const auto& cardIdxElm : deck.get_array().value) {
        if (cardIdxElm.type() != bsoncxx::type::k_int32) {
            return false;
        }
        auto card = world_->GetCard(cardIdxElm.get_int32().value);
        if (!card) {
            return false;
        }
        deck_.push_back(card);
    }
    const auto& turnsElm = bson["moves"];
    if (turnsElm.type() != bsoncxx::type::k_array) {
        return false;
    }
    for (const auto& turnElm : turnsElm.get_array().value) {
        if (turnElm.type() != bsoncxx::type::k_document) {
            return false;
        }
        auto user = turnElm["user"];
        if (!user || user.type() != bsoncxx::type::k_utf8) {
            return false;
        }
        auto move = std::make_shared<Move>(std::string(user.get_utf8().value));
        move->FromJson(turnElm.get_document().view());
        moves_.push_back(move);
    }
    return true;
}

void Game::ToJson(bsoncxx::builder::stream::document& d, std::string_view forUser /* = ""*/) const
{
    d << "name" << name_;
    d << "state" << stateToString(turnState_);
    auto t = d << "closet";
    closet_->ToJson(t);
    t = d << "turn";
    auto activePlayer = getActivePlayer();
    if (!activePlayer) {
        t << "";
    } else {
        t << activePlayer->GetUser();
    }
    auto arr = d << "players" << bsoncxx::builder::stream::open_array;
    for (const auto& player : players_) {
        bool hidden = !forUser.empty() && player->GetUser() != forUser;
        bsoncxx::builder::stream::document pd;
        player->ToJson(pd, hidden);
        arr << pd;
    }
    arr << bsoncxx::builder::stream::close_array;

    auto t2 = d << "deck";
    if (forUser.empty()) {
        auto arr2 = t2 << bsoncxx::builder::stream::open_array;
        for (auto card : deck_) {
            arr << card->GetID();
        }
        arr2 << bsoncxx::builder::stream::close_array;
    } else {
        t2 << static_cast<int64_t>(deck_.size());
    }
    d << "updated" << lastMove_;
}

const std::string& Game::GetName() const
{
    return name_;
}

bool Game::AddPlayer(std::string& user)
{
    if (players_.size() >= world_->GetRules()->MaxPlayers) {
        return false;
    }
    if (turnState_ != TurnState::Preparing) {
        return false;
    }
    bool alreadyHas =
        std::any_of(players_.cbegin(), players_.cend(), [&](const auto& player) { return player->GetUser() == user; });
    if (alreadyHas) {
        return false;
    }
    auto player = std::make_unique<Player>(world_.get(), user);
    players_.push_back(std::move(player));
    return true;
}

std::string Game::stateToString(TurnState state) const
{
    switch (state) {
        case TurnState::Preparing:
            return "preparing";
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

int64_t Game::LastUpdated() const
{
    return lastMove_;
}

const std::list<std::shared_ptr<Move>> Game::GetMoves() const
{
    return moves_;
}
} // namespace Engine
