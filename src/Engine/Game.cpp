#include <assert.h>
#include <sstream>

#include "Game.h"

#include "Utils/Utils.h"
namespace Engine
{
void Game::SpecialState::ToJson(
    bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d) const
{
    bsoncxx::builder::stream::document res;
    res << "state" << std::string(StateNames.at(State));
    res << "player" << static_cast<int>(PlayerIdx);
    res << "drawremains" << DrawRemains;
    res << "ingredient" << IngredientRequested;
    d << res;
}

bool Game::SpecialState::FromJson(const bsoncxx::document::view& bson)
{
    const auto& state = bson["state"];
    if (!state || state.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string stateStr(state.get_utf8().value);
    bool found = false;
    for (const auto& pair : StateNames) {
        if (pair.second == stateStr) {
            State = pair.first;
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }
    const auto& player = bson["player"];
    if (!player || player.type() != bsoncxx::type::k_int32) {
        return false;
    }
    PlayerIdx = static_cast<size_t>(player.get_int32().value);
    const auto& drawRemains = bson["drawremains"];
    if (!drawRemains || drawRemains.type() != bsoncxx::type::k_int32) {
        return false;
    }
    DrawRemains = drawRemains.get_int32().value;
    const auto& ing = bson["ingredient"];
    if (!ing || ing.type() != bsoncxx::type::k_int32) {
        return false;
    }
    IngredientRequested = ing.get_int32().value;
    return true;
}

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
    if (expansions_ > 0) {
        closet_->ActivateExpansion();
    }
    return true;
}

Card* Game::getTopCard(World::DeckType type)
{
    if (decks_.count(type) == 0) {
        return nullptr;
    }
    auto& deck = decks_.at(type);
    if (deck.empty()) {
        return nullptr;
    }
    auto card = deck.back();
    deck.pop_back();
    return card;
}

void Game::drawCard(World::DeckType type)
{
    auto p = getActivePlayer();
    auto card = getTopCard(type);
    p->AddCard(card);
    if (specialState_.State == SpecialState::StateType::DrawExtra ||
        p->HandSize() >= world_->GetRules()->MinCardsInHand) {
        advanceState();
    }
}

void Game::endTurn()
{
    assert(turnState_ == TurnState::Done);
    checkIncomeTalisman();
    ++activePlayerIdx_;
    if (activePlayerIdx_ >= players_.size()) {
        activePlayerIdx_ = 0;
    }
    checkGrowthTalisman();
    advanceState();
}

void Game::discardCard(Card* card)
{
    assert(turnState_ == TurnState::Playing || turnState_ == TurnState::DrawPlaying);
    auto p = getActivePlayer();
    bool willScore =
        !closet_->HasIngredient(card->GetIngredient()) && !world_->HasUniversalIngredient(card);
    if (willScore || p->HasTalisman(Card::TalismanType::Usefulness)) {
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

void Game::advanceSpecialState()
{
    switch (specialState_.State) {
        case SpecialState::StateType::Giving:
            if (specialState_.PlayerIdx == activePlayerIdx_) {
                specialState_.State = SpecialState::StateType::None;
            }
            ++specialState_.PlayerIdx;
            if (specialState_.PlayerIdx >= players_.size()) {
                specialState_.PlayerIdx = 0;
            }
            break;
        case SpecialState::StateType::Disassembling:
            ++specialState_.PlayerIdx;
            if (specialState_.PlayerIdx >= players_.size()) {
                specialState_.PlayerIdx = 0;
            }
            if (specialState_.PlayerIdx == activePlayerIdx_) {
                specialState_.State = SpecialState::StateType::None;
                advanceState();
            }
            break;
        case SpecialState::StateType::Transfiguring:
            specialState_.State = SpecialState::StateType::None;
            advanceState();
            break;
        case SpecialState::StateType::DrawExtra:
            --specialState_.DrawRemains;
            if (specialState_.DrawRemains == 0) {
                specialState_.State = SpecialState::StateType::None;
                advanceState();
            }
            break;
        default:
            assert(0);
            break;
    }
}

void Game::checkGrowthTalisman()
{
    auto player = getActivePlayer();
    if (!player->HasTalisman(Card::TalismanType::Growth)) {
        return;
    }
    for (const auto& playerItr : players_) {
        if (playerItr.get() == player) {
            continue;
        }
        if (playerItr->GetScore() > player->GetScore()) {
            player->AddScore(1);
            return;
        }
    }
}

void Game::checkIncomeTalisman()
{
    auto player = getActivePlayer();
    if (!player->HasTalisman(Card::TalismanType::Income)) {
        return;
    }
    for (const auto& playerItr : players_) {
        if (playerItr.get() == player) {
            continue;
        }
        if (playerItr->AssembledSize() > player->AssembledSize()) {
            player->AddScore(1);
            return;
        }
    }
}

void Game::advanceState()
{
    if (specialState_.State != SpecialState::StateType::None) {
        advanceSpecialState();
        return;
    }
    if (extraPlayMoves_ > 0) {
        --extraPlayMoves_;
        if (extraPlayMoves_ > 0) {
            return;
        }
    }
    switch (turnState_) {
        case TurnState::Drawing:
            if (expansions_ > 0) {
                turnState_ = TurnState::DrawPlaying;
            } else {
                turnState_ = TurnState::Playing;
            }
            break;
        case TurnState::DrawPlaying:
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
    auto p = getActivePlayer();
    assert(p->HasCard(card));
    std::map<Player*, int> playersToScore;
    bool usingUniversal = false;
    for (auto part : parts) {
        if (part->IsAssembled()) {
            if (world_->IsUniversalRecipe(part)) {
                usingUniversal = true;
            }
            std::for_each(players_.begin(), players_.end(), [&](auto& player) {
                if (!player->HasAssembled(part)) {
                    return;
                }
                bool hasTalisman = player->HasTalisman(Card::TalismanType::Generality);
                player->RemoveAssembled(part);
                if (hasTalisman) {
                    ++playersToScore[player.get()];
                } else if (!part->UsingUniversal()) {
                    playersToScore[player.get()] = 1;
                }
            });
            for (auto assembledPart : part->GetParts()) {
                closet_->AddCard(assembledPart);
            }
            part->Disassemble();
        } else {
            if (world_->HasUniversalIngredient(part)) {
                usingUniversal = true;
            }
            closet_->RemoveCard(part);
        }
    }
    playersToScore[p] = 1;
    for (auto& [player, num] : playersToScore) {
        if (player == p) {
            player->AddScore(card->GetScore());
        } else {
            player->AddScore(num * card->GetScore() / 2);
        }
    }
    card->Assemble(parts, usingUniversal);
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
    auto prepareDeck = [&](World::DeckType type, size_t closetSize, size_t handSize) {
        world_->PrepareDeck(decks_[type], type);
        for (size_t i = 0; i < closetSize; ++i) {
            auto card = getTopCard(type);
            closet_->AddCard(card);
        }
        for (size_t i = 0; i < handSize; ++i) {
            for (const auto& p : players_) {
                auto card = getTopCard(type);
                p->AddCard(card);
            }
        }
    };
    prepareDeck(World::DeckType::Base, world_->GetRules()->InitialClosetSize,
        world_->GetRules()->InitialHandSize);
    if (hasExpansion(World::DeckType::University)) {
        prepareDeck(World::DeckType::University, world_->GetRules()->InitialExpansionClosetSize,
            world_->GetRules()->InitialExpansionHandSize);
    }
    if (hasExpansion(World::DeckType::Guild)) {
        prepareDeck(World::DeckType::Guild, world_->GetRules()->InitialExpansionClosetSize,
            world_->GetRules()->InitialExpansionHandSize);
    }
    turnState_ = TurnState::Drawing;
}

bool Game::validateDisassemble(const Move& move) const
{
    if (specialState_.State != SpecialState::StateType::Disassembling) {
        return false;
    }
    auto specialPlayer = players_[specialState_.PlayerIdx].get();
    if (move.GetUser() != specialPlayer->GetUser()) {
        return false;
    }
    auto card = world_->GetCard(move.GetCard());
    if (!specialPlayer->HasAssembled(card)) {
        return false;
    }
    if (card->GetParts().empty()) {
        return false;
    }
    return true;
}

bool Game::validateSpecialEndTurn(const Move& move) const
{
    auto player = players_[specialState_.PlayerIdx].get();
    if (player->GetUser() != move.GetUser()) {
        return false;
    }
    if (specialState_.State == SpecialState::StateType::DrawExtra) {
        return true;
    } else if (specialState_.State == SpecialState::StateType::Disassembling) {
        return !player->HasAssembledCardWithParts();
    } else if (specialState_.State == SpecialState::StateType::Giving) {
        return !player->HasCardWithIngredient(specialState_.IngredientRequested);
    }
    return false;
}

bool Game::validateSpecialDiscard(const Move& move) const
{
    if (specialState_.State != SpecialState::StateType::Giving) {
        return false;
    }
    auto player = players_[specialState_.PlayerIdx].get();
    if (player->GetUser() != move.GetUser()) {
        return false;
    }
    auto card = world_->GetCard(move.GetCard());
    if (!player->HasCard(card)) {
        return false;
    }
    if (card->GetIngredient() != specialState_.IngredientRequested) {
        return false;
    }
    return true;
}

bool Game::validateSpecialAssembly(const Move& move) const
{
    auto player = getActivePlayer();
    if (player->GetUser() != move.GetUser()) {
        return false;
    }
    auto card = world_->GetCard(move.GetCard());
    if (!player->HasCard(card)) {
        return false;
    }
    auto parts = move.GetParts(world_.get());
    if (std::any_of(parts.begin(), parts.end(), [&](const auto& part) {
            return !part->IsAssembled() && !closet_->CanRemoveCard(part);
        })) {
        return false;
    }
    if (!card->CanAssemble(parts, 1)) {
        return false;
    }
    return true;
}

bool Game::validateSpecialMove(const Move& move) const
{
    switch (move.GetAction()) {
        case Move::Action::Disassemble:
            return validateDisassemble(move);
        case Move::Action::EndTurn:
        case Move::Action::Skip:
            return validateSpecialEndTurn(move);
        case Move::Action::Discard:
            return validateSpecialDiscard(move);
        case Move::Action::Assemble:
            return validateSpecialAssembly(move);
        case Move::Action::Draw:
            return specialState_.State == SpecialState::StateType::DrawExtra &&
                   specialState_.DrawRemains > 0;
        default:
            break;
    }
    return false;
}

bool Game::ValidateMove(const Move* move) const
{
    if (specialState_.State != SpecialState::StateType::None) {
        return validateSpecialMove(*move);
    }
    if (move->IsService()) {
        return true;
    }
    auto activePlayer = getActivePlayer();
    if (activePlayer->GetUser() != move->GetUser()) {
        return false;
    }
    switch (move->GetAction()) {
        case Move::Action::Draw:
            return validateDraw(*move);
        case Move::Action::Skip:
            return validateSkip();
        case Move::Action::Discard:
            return validateDiscard(*move);
        case Move::Action::Assemble:
            return validateAssemble(*move);
        case Move::Action::Cast:
            return validateCast(*move);
        case Move::Action::EndTurn:
            return turnState_ == TurnState::Done;
        default:
            break;
    }
    return false;
}

void Game::performDisassemble(const Move& move)
{
    auto card = world_->GetCard(move.GetCard());
    for (auto part : card->GetParts()) {
        closet_->AddCard(part);
    }
    card->Disassemble(true);
}

void Game::performSpecialDiscard(const Move& move)
{
    auto player = players_[specialState_.PlayerIdx].get();
    auto card = world_->GetCard(move.GetCard());
    player->DiscardCard(card);
    closet_->AddCard(card);
    specialState_.State = SpecialState::StateType::None;
}

void Game::performSpecialAssemble(const Move& move)
{
    auto card = world_->GetCard(move.GetCard());
    auto parts = move.GetParts(world_.get());
    assemble(card, parts);
}

void Game::performSpecialMove(std::shared_ptr<Move>& move)
{
    switch (move->GetAction()) {
        case Move::Action::Disassemble:
            performDisassemble(*move);
            break;
        case Move::Action::Skip:
        case Move::Action::EndTurn:
            advanceState();
            break;
        case Move::Action::Discard:
            performSpecialDiscard(*move);
            break;
        case Move::Action::Assemble:
            performSpecialAssemble(*move);
            break;
        case Move::Action::Draw:
            drawCard(world_->DeckFromString(move->GetDeckType()));
            break;
        default:
            assert(0);
    }
    lastMove_ = Utils::GetTime();
    moves_.push_back(move);
}

void Game::PerformMove(std::shared_ptr<Move>& move)
{
    assert(ValidateMove(move.get()));
    if (specialState_.State != SpecialState::StateType::None) {
        performSpecialMove(move);
        return;
    }
    switch (move->GetAction()) {
        case Move::Action::Draw:
            drawCard(world_->DeckFromString(move->GetDeckType()));
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
        case Move::Action::OrganizeHand:
            organizeHand(*move);
            return;
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

void Game::performCastWhirpool(const Move& move)
{
    specialState_.State = SpecialState::StateType::Disassembling;
    specialState_.PlayerIdx = activePlayerIdx_;
    auto player = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    player->DiscardCard(card);
    closet_->AddCard(card);
}

void Game::performCastNecessity(const Move& move)
{
    specialState_.State = SpecialState::StateType::Giving;
    specialState_.IngredientRequested = move.GetIngredient();
    specialState_.PlayerIdx = activePlayerIdx_ + 1;
    if (specialState_.PlayerIdx >= players_.size()) {
        specialState_.PlayerIdx = 0;
    }
    auto player = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    player->DiscardCard(card);
    closet_->AddCard(card);
}

void Game::performCastMagicReveal(const Move& move)
{
    auto player = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    auto cardToReveal = move.GetParts(world_.get())[0];
    closet_->RemoveCard(cardToReveal);
    player->DiscardCard(card);
    closet_->AddCard(card);
    player->AddCard(cardToReveal);
}

void Game::performCastCreate(const Move& move)
{
    auto player = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    auto cardToCreate = move.GetParts(world_.get())[0];
    if (player->HasCard(cardToCreate)) {
        player->DiscardCard(cardToCreate);
    } else {
        assert(closet_->CanRemoveCard(cardToCreate));
        closet_->RemoveCard(cardToCreate);
    }
    cardToCreate->Assemble({});
    player->AddAssembled(cardToCreate);
    player->DiscardCard(card);
    closet_->AddCard(card);
    advanceState();
}

void Game::performCastTransfigure(const Move& move)
{
    auto card = world_->GetCard(move.GetCard());
    auto player = getActivePlayer();
    player->DiscardCard(card);
    closet_->AddCard(card);
    specialState_.State = SpecialState::StateType::Transfiguring;
    specialState_.PlayerIdx = activePlayerIdx_;
}

void Game::performCastOverthrow(const Move& move)
{
    auto card = world_->GetCard(move.GetCard());
    auto parts = move.GetParts(world_.get());
    auto player = getActivePlayer();
    player->DiscardCard(card);
    closet_->AddCard(card);
    for (const auto& p : players_) {
        auto c = p->FindAssembledWithPart(parts[0]);
        if (!c) {
            continue;
        }
        auto cardParts = c->GetParts();
        for (auto cardPart : cardParts) {
            if (cardPart != parts[0]) {
                closet_->AddCard(cardPart);
            }
        }
        c->Disassemble();
        p->RemoveAssembled(c);
        closet_->AddCard(c);
        player->AddCard(parts[0]);
        break;
    }
    advanceState();
}

void Game::performCastDiversity(const Move& move)
{
    specialState_.State = SpecialState::StateType::DrawExtra;
    specialState_.PlayerIdx = activePlayerIdx_;
    specialState_.DrawRemains = 3;
    auto card = world_->GetCard(move.GetCard());
    auto player = getActivePlayer();
    player->DiscardCard(card);
    closet_->AddCard(card);
}

void Game::performCastForest(const Move& move)
{
    auto card = world_->GetCard(move.GetCard());
    auto parts = move.GetParts(world_.get());
    auto player = getActivePlayer();
    player->DiscardCard(card);
    closet_->AddCard(card);
    for (auto critter : parts) {
        for (const auto& pItr : players_) {
            if (!pItr->HasAssembled(critter)) {
                continue;
            }
            if (pItr.get() != player) {
                pItr->AddScore(2);
            }
            pItr->RemoveAssembled(critter);
            for (auto critterPart : critter->GetParts()) {
                closet_->AddCard(critterPart);
            }
            critter->Disassemble();
            closet_->AddCard(critter);
        }
    }
    player->AddScore(10);
    advanceState();
}

void Game::performCastConcience(const Move& move)
{
    auto activePlayer = getActivePlayer();
    Player* targetPlayer = findPlayer(move.GetPlayer());
    assert(targetPlayer);
    targetPlayer->AddScore(-5);
    auto card = world_->GetCard(move.GetCard());
    activePlayer->DiscardCard(card);
    targetPlayer->AddCard(card);
    advanceState();
}

void Game::performCastSpeedup(const Move& move)
{
    auto activePlayer = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    activePlayer->DiscardCard(card);
    extraPlayMoves_ = 2;
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
        case 77:
        case 78:
        case 79:
            performCastWhirpool(move);
            break;
        case 80:
        case 81:
        case 82:
            performCastNecessity(move);
            break;
        case 83:
        case 84:
        case 85:
            performCastMagicReveal(move);
            advanceState();
            break;
        case 86:
        case 87:
        case 88:
            performCastCreate(move);
            break;
        case 89:
        case 90:
        case 91:
            performCastMagicReveal(move);
            break;
        case 92:
        case 93:
        case 94:
            performCastTransfigure(move);
            break;
        case 95:
        case 96:
        case 97:
            performCastOverthrow(move);
            break;
        case 98:
        case 99:
        case 100:
            performCastDiversity(move);
            break;
        case 101:
        case 102:
        case 103:
            performCastSpeedup(move);
            break;
        case 104:
        case 105:
        case 106:
            performCastForest(move);
            break;
        case 134:
        case 135:
        case 136:
            performCastConcience(move);
            break;
    }
}

bool Game::validateAssemble(const Move& move) const
{
    if (turnState_ != TurnState::DrawPlaying && turnState_ != TurnState::Playing) {
        return false;
    }
    auto parts = move.GetParts(world_.get());
    for (auto part : parts) {
        if (!part->IsAssembled()) {
            if (!closet_->CanRemoveCard(part)) {
                return false;
            }
        }
    }
    return world_->GetCard(move.GetCard())->CanAssemble(parts);
}

bool Game::validateDiscard(const Move& move) const
{
    auto activePlayer = getActivePlayer();
    if (turnState_ != TurnState::DrawPlaying && turnState_ != TurnState::Playing) {
        return false;
    }
    return activePlayer->HasCard(world_->GetCard(move.GetCard()));
}

bool Game::validateSkip() const
{
    auto activePlayer = getActivePlayer();
    bool decksEmpty = true;
    for (const auto& pair : decks_) {
        if (!pair.second.empty()) {
            decksEmpty = false;
            break;
        }
    }
    if (turnState_ == TurnState::Drawing) {
        if (activePlayer->HandSize() >= world_->GetRules()->MaxHandToDraw || decksEmpty) {
            return true;
        }
    }
    if (turnState_ == TurnState::DrawPlaying && activePlayer->HandSize() == 0 && decksEmpty) {
        return true;
    }
    if (turnState_ == TurnState::Playing && activePlayer->HandSize() == 0) {
        return true;
    }
    return false;
}

bool Game::validateDraw(const Move& move) const
{
    if (turnState_ != TurnState::Drawing && turnState_ != TurnState::DrawPlaying) {
        return false;
    }
    if (extraPlayMoves_ > 0) {
        return false;
    }
    auto activePlayer = getActivePlayer();
    if (activePlayer->HandSize() >= world_->GetRules()->MaxHandToDraw) {
        return false;
    }
    const auto& deck = decks_.at(world_->DeckFromString(move.GetDeckType()));
    if (deck.empty()) {
        return false;
    }
    return true;
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

bool Game::validateCastGlobalReveal(const Move& move) const
{
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    auto cardToReveal = parts[0];
    if (!closet_->CanRemoveCard(cardToReveal)) {
        return false;
    }
    return true;
}

bool Game::validateCastCreate(const Move& move) const
{
    auto player = getActivePlayer();
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    auto cardToCreate = parts[0];
    if (cardToCreate->GetType() != Card::Type::Recipe) {
        return false;
    }
    if (!player->HasCard(cardToCreate) && !closet_->CanRemoveCard(cardToCreate)) {
        return false;
    }
    return true;
}

bool Game::validateCastMagicReveal(const Move& move) const
{
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    auto cardToReveal = parts[0];
    if (cardToReveal->GetType() != Card::Type::Spell) {
        return false;
    }
    if (!closet_->CanRemoveCard(cardToReveal)) {
        return false;
    }
    return true;
}

bool Game::validateCastOverthrow(const Move& move) const
{
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 1) {
        return false;
    }
    for (const auto& player : players_) {
        auto card = player->FindAssembledWithPart(parts[0]);
        if (!card) {
            continue;
        }
        if (world_->IsCritter(card)) {
            return true;
        }
        auto id = card->GetID();
        if (id == 69 || id == 70 || id == 137 || id == 138 || id == 139) {
            return true;
        }
        return false;
    }
    return false;
}

bool Game::validateCastForest(const Move& move) const
{
    auto parts = move.GetParts(world_.get());
    if (parts.size() != 2) {
        return false;
    }
    for (auto critter : parts) {
        if (!world_->IsCritter(critter)) {
            return false;
        }
        bool found = false;
        for (const auto& player : players_) {
            if (player->HasAssembled(critter)) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool Game::validateCastConcience(const Move& move) const
{
    auto activePlayer = getActivePlayer();
    auto card = world_->GetCard(move.GetCard());
    if (!activePlayer->HasCard(card)) {
        return false;
    }
    Player* targetPlayer = findPlayer(move.GetPlayer());
    if (!targetPlayer) {
        return false;
    }
    if (targetPlayer->GetScore() <= activePlayer->GetScore()) {
        return false;
    }
    return true;
}

bool Game::validateCast(const Move& move) const
{
    if (turnState_ != TurnState::Playing && turnState_ != TurnState::DrawPlaying) {
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
        case 77:
        case 78:
        case 79:
            return true;
        case 80:
        case 81:
        case 82:
            return move.GetIngredient() >= 0 && move.GetIngredient() < 16;
        case 83:
        case 84:
        case 85:
            return validateCastGlobalReveal(move);
        case 86:
        case 87:
        case 88:
            return validateCastCreate(move);
        case 89:
        case 90:
        case 91:
            return validateCastMagicReveal(move);
        case 92:
        case 93:
        case 94:
            return true;
        case 95:
        case 96:
        case 97:
            return validateCastOverthrow(move);
        case 98:
        case 99:
        case 100:
            return true;
        case 101:
        case 102:
        case 103:
            return true;
        case 104:
        case 105:
        case 106:
            return validateCastForest(move);
        case 134:
        case 135:
        case 136:
            return validateCastConcience(move);
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
    const auto extraPlay = state["extraplaymoves"];
    if (!extraPlay || extraPlay.type() != bsoncxx::type::k_int32) {
        return false;
    }
    extraPlayMoves_ = extraPlay.get_int32().value;
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
    } else if (stateStr == "drawplaying") {
        turnState_ = TurnState::DrawPlaying;
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
    const auto& specialState = state["specialstate"];
    if (!specialState || specialState.type() != bsoncxx::type::k_document) {
        return false;
    }
    res = specialState_.FromJson(specialState.get_document().view());
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
    const auto& decks = state["decks"];
    if (!decks || decks.type() != bsoncxx::type::k_document) {
        return false;
    }
    for (const auto& deckArray : decks.get_document().value) {
        if (deckArray.type() != bsoncxx::type::k_array) {
            return false;
        }
        std::string deckIdxStr(deckArray.key());
        auto type = world_->DeckFromString(deckIdxStr);
        if (type == World::DeckType::Unknown) {
            return false;
        }
        auto& deck = decks_[type];
        for (const auto& cardIdxElm : deckArray.get_array().value) {
            if (cardIdxElm.type() != bsoncxx::type::k_int32) {
                return false;
            }
            auto card = world_->GetCard(cardIdxElm.get_int32().value);
            if (!card) {
                return false;
            }
            deck.push_back(card);
        }
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
        if (!move->FromJson(turnElm.get_document().view())) {
            return false;
        }
        moves_.push_back(move);
    }
    const auto& expansions = bson["expansions"];
    if (!expansions || expansions.type() != bsoncxx::type::k_int32) {
        return false;
    }
    expansions_ = expansions.get_int32().value;
    if (expansions_ > 0) {
        world_->ActivateExpansion();
        closet_->ActivateExpansion();
    }
    return true;
}

void Game::ToJson(bsoncxx::builder::stream::document& d, std::string_view forUser /* = ""*/) const
{
    d << "name" << name_;
    d << "state" << std::string(turnStateNames_.at(turnState_));
    auto specialStateBson = d << "specialstate";
    specialState_.ToJson(specialStateBson);
    d << "extraplaymoves" << extraPlayMoves_;
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

    bsoncxx::builder::stream::document decksBson;
    for (const auto& pair : decks_) {
        auto t2 = decksBson << world_->DeckToString(pair.first);
        if (forUser.empty()) {
            auto arr2 = t2 << bsoncxx::builder::stream::open_array;
            for (auto card : pair.second) {
                arr2 << card->GetID();
            }
            arr2 << bsoncxx::builder::stream::close_array;
        } else {
            t2 << static_cast<int64_t>(pair.second.size());
        }
    }
    d << "decks" << decksBson;
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
    bool alreadyHas = std::any_of(players_.cbegin(), players_.cend(),
        [&](const auto& player) { return player->GetUser() == user; });
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
        case TurnState::DrawPlaying:
            return "drawplaying";
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

void Game::ActivateExpansion(World::DeckType type)
{
    expansions_ |= Utils::enum_value(type);
    world_->ActivateExpansion();
}

bool Game::hasExpansion(World::DeckType type)
{
    return expansions_ & Utils::enum_value(type);
}

int Game::GetExpansions() const
{
    return expansions_;
}

Player* Game::findPlayer(const std::string& user) const
{
    Player* res = nullptr;
    for (const auto& itr : players_) {
        if (itr->GetUser() == user) {
            res = itr.get();
            break;
        }
    }
    return res;
}

void Game::organizeHand(const Move& move)
{
    for (auto& p : players_) {
        if (p->GetUser() == move.GetUser()) {
            const auto& cards = move.GetParts(world_.get());
            if (cards.size() != 1) {
                return;
            }
            p->OrganizeHand(world_->GetCard(move.GetCard()), cards[0]);
            break;
        }
    }
}
} // namespace Engine
