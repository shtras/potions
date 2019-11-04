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

bool Game::ValidateMove(const Move& move) const
{
    auto activePlayer = getActivePlayer();
    if (activePlayer->GetUser() != move.GetUser()) {
        return false;
    }
    switch (move.GetAction()) {
        case Move::Action::Draw:
            return turnState_ == TurnState::Drawing && activePlayer->HandSize() < world_->GetRules()->MaxHandToDraw &&
                   !deck_.empty();
        case Move::Action::Skip:
            return turnState_ == TurnState::Drawing &&
                   (activePlayer->HandSize() >= world_->GetRules()->MaxHandToDraw || deck_.empty());
        case Move::Action::Discard:
            return turnState_ == TurnState::Playing && activePlayer->HasCard(world_->GetCard(move.GetCard()));
        case Move::Action::Assemble:
            return turnState_ == TurnState::Playing &&
                   world_->GetCard(move.GetCard())->CanAssemble(move.GetParts(world_.get()));
        case Move::Action::Cast:
            return validateCast(move);
        case Move::Action::EndTurn:
            return turnState_ == TurnState::Done;
        default:
            assert(0);
    }
    return false;
}

void Game::PerformMove(const Move& move)
{
    assert(ValidateMove(move));
    switch (move.GetAction()) {
        case Move::Action::Draw:
            drawCard();
            break;
        case Move::Action::Skip:
            advanceState();
            break;
        case Move::Action::Discard:
            discardCard(world_->GetCard(move.GetCard()));
            break;
        case Move::Action::Assemble:
            assemble(world_->GetCard(move.GetCard()), move.GetParts(world_.get()));
            break;
        case Move::Action::Cast:
            performCast(move);
            break;
        case Move::Action::EndTurn:
            endTurn();
            break;
        default:
            assert(0);
    }
    lastMove_ = Utils::GetTime();
}

void Game::performCastTransform(const Move& move)
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    auto card = world_->GetCard(move.GetCard());
    Card* cardFromCloset = nullptr;
    Card* cardFromAssembled = nullptr;
    if (player->HasAssembled(parts[0])) {
        cardFromCloset = parts[1];
        cardFromAssembled = parts[0];
    } else {
        cardFromCloset = parts[0];
        cardFromAssembled = parts[1];
    }
    closet_->RemoveCard(cardFromCloset);
    closet_->AddCard(card);
    std::vector<Card*> v;
    cardFromCloset->Assemble(v);
    player->AddAssembled(cardFromCloset);
    for (auto part : cardFromAssembled->GetParts()) {
        closet_->AddCard(part);
    }
    cardFromAssembled->Disassemble();
    closet_->AddCard(cardFromAssembled);
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
        return true;
    }
    if (closet_->CanRemoveCard(parts[1]) && player->HasAssembled(parts[0])) {
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
    return true;
}

bool Game::validateCastDestroy(const Move& move) const
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
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
    auto closetO = Utils::GetT<rapidjson::Value::ConstObject>(d, "closet");
    if (!closetO) {
        return false;
    }
    closet_ = std::make_unique<Closet>(world_.get());
    bool res = closet_->FromJson(*closetO);
    if (!res) {
        return false;
    }
    auto playersO = Utils::GetT<rapidjson::Value::ConstArray>(d, "players");
    if (!playersO) {
        return false;
    }
    const auto& players = *playersO;
    for (rapidjson::SizeType i = 0; i < players.Size(); ++i) {
        auto playerO = Utils::GetT<rapidjson::Value::ConstObject>(players[i]);
        if (!playerO) {
            return false;
        }
        const auto& player = *playerO;
        if (!player.HasMember("user") || !player["user"].IsString()) {
            return false;
        }
        std::string user = player["user"].GetString();
        if (user == turn) {
            activePlayerIdx_ = i;
        }
        auto p = std::make_unique<Player>(world_.get(), user);
        if (!p->FromJson(player)) {
            return false;
        }
        players_.push_back(std::move(p));
    }
    auto deckO = Utils::GetT<rapidjson::Value::ConstArray>(d, "deck");
    if (!deckO) {
        return false;
    }
    const auto& deck = *deckO;
    for (rapidjson::SizeType i = 0; i < deck.Size(); ++i) {
        auto cardIdxO = Utils::GetT<int>(deck[i]);
        if (!cardIdxO) {
            return false;
        }
        auto card = world_->GetCard(*cardIdxO);
        if (!card) {
            return false;
        }
        deck_.push_back(card);
    }
    return true;
}

void Game::ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, std::string_view forUser /* = ""*/) const
{
    w.StartObject();
    w.Key("name");
    w.String(name_);
    w.Key("state");
    w.String(stateToString(turnState_));
    w.Key("closet");
    closet_->ToJson(w);
    w.Key("turn");
    auto activePlayer = getActivePlayer();
    if (!activePlayer) {
        w.String("");
    } else {
        w.String(activePlayer->GetUser());
    }
    w.Key("players");
    w.StartArray();
    for (const auto& player : players_) {
        bool hidden = !forUser.empty() && player->GetUser() != forUser;
        player->ToJson(w, hidden);
    }
    w.EndArray();
    w.Key("deck");
    if (forUser.empty()) {
        w.StartArray();
        for (auto card : deck_) {
            w.Int(card->GetID());
        }
        w.EndArray();
    } else {
        w.Uint64(deck_.size());
    }
    w.Key("updated");
    w.Int64(lastMove_);
    w.EndObject();
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
} // namespace Engine
