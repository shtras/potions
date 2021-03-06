#include "Utils/Utils.h"

#include "Player.h"

namespace Engine
{
Player::Player(World* w, std::string& user)
    : world_(w)
    , user_(user)
{
}

void Player::AddCard(Card* card)
{
    hand_.push_back(card);
}

size_t Player::HandSize() const
{
    return hand_.size();
}

size_t Player::AssembledSize() const
{
    return assembledCards_.size();
}

void Player::DiscardCard(Card* card)
{
    assert(HasCard(card));
    hand_.remove(card);
}

bool Player::FromJson(const bsoncxx::document::view& bson)
{
    const auto& hand = bson["hand"];
    if (hand.type() != bsoncxx::type::k_array) {
        return false;
    }
    for (const auto& elm : hand.get_array().value) {
        if (elm.type() != bsoncxx::type::k_int32) {
            return false;
        }
        auto idx = elm.get_int32().value;
        auto card = world_->GetCard(idx);
        hand_.push_back(card);
    }
    const auto& score = bson["score"];
    if (score.type() != bsoncxx::type::k_int32) {
        return false;
    }
    score_ = score.get_int32().value;
    const auto& table = bson["table"];
    if (table.type() != bsoncxx::type::k_document) {
        return false;
    }
    for (const auto& elm : table.get_document().value) {
        auto cardIdx = std::stoi(std::string(elm.key()));
        auto card = world_->GetCard(cardIdx);
        if (!card) {
            return false;
        }
        if (elm.type() != bsoncxx::type::k_document) {
            return false;
        }
        const auto& tableCard = elm.get_document().value;
        const auto& universal = tableCard["universal"];
        bool usingUniversal = false;
        if (universal) {
            if (universal.type() != bsoncxx::type::k_bool) {
                return false;
            }
            usingUniversal = universal.get_bool().value;
        }
        const auto parts = tableCard["parts"];
        if (!parts || parts.type() != bsoncxx::type::k_array) {
            return false;
        }
        std::vector<Card*> partsVec;
        for (const auto& part : parts.get_array().value) {
            if (part.type() != bsoncxx::type::k_int32) {
                return false;
            }
            partsVec.push_back(world_->GetCard(part.get_int32().value));
        }
        card->Assemble(partsVec, usingUniversal);
        assembledCards_.insert(card);
    }
    refreshTalismans();
    return true;
}

Card* Player::FindAssembledWithPart(Card* part) const
{
    for (auto card : assembledCards_) {
        if (card->HasPart(part)) {
            return card;
        }
    }
    return nullptr;
}

void Player::ToJson(bsoncxx::builder::stream::document& d, bool hidden /* = false*/) const
{
    d << "user" << user_;
    d << "score" << score_;
    auto t = d << "hand";
    if (hidden) {
        std::map<World::DeckType, int> handCards = {{World::DeckType::Base, 0},
            {World::DeckType::University, 0}, {World::DeckType::Guild, 0}};
        for (const auto& card : hand_) {
            auto type = world_->GetCardType(card);
            ++handCards.at(type);
        }
        bsoncxx::builder::stream::document handBson;
        for (const auto& pair : handCards) {
            if (pair.second > 0) {
                handBson << world_->DeckToString(pair.first) << pair.second;
            }
        }
        t << handBson;
    } else {
        bsoncxx::builder::stream::array a;
        for (auto card : hand_) {
            a << card->GetID();
        }
        t << a;
    }
    auto t1 = d << "table";
    bsoncxx::builder::stream::document d2;
    for (auto card : assembledCards_) {
        bsoncxx::builder::stream::array a;
        for (const auto& part : card->GetParts()) {
            a << part->GetID();
        }
        d2 << std::to_string(card->GetID()) << bsoncxx::builder::stream::open_document
           << "universal" << card->UsingUniversal() << "parts" << a
           << bsoncxx::builder::stream::close_document;
    }
    t1 << d2;
}

bool Player::HasCard(const Card* card) const
{
    return std::find(hand_.begin(), hand_.end(), card) != hand_.end();
}

void Player::AddAssembled(Card* card)
{
    assembledCards_.insert(card);
    refreshTalismans();
}

void Player::RemoveAssembled(Card* card)
{
    assert(assembledCards_.count(card) > 0);
    assembledCards_.erase(card);
    refreshTalismans();
}

const std::string& Player::GetUser() const
{
    return user_;
}

bool Player::HasAssembled(Card* card) const
{
    return assembledCards_.count(card) > 0;
}

void Player::AddScore(int score)
{
    score_ += score;
}

bool Player::HasCardWithIngredient(int id) const
{
    return std::any_of(
        hand_.begin(), hand_.end(), [&id](const auto& c) { return c->GetIngredient() == id; });
}

bool Player::HasAssembledCardWithParts() const
{
    return std::any_of(assembledCards_.cbegin(), assembledCards_.cend(),
        [](const auto& c) { return !c->GetParts().empty(); });
}

bool Player::HasTalisman(Card::TalismanType type) const
{
    return talismans_.count(type) > 0;
}

void Player::refreshTalismans()
{
    talismans_.clear();
    for (const auto& c : assembledCards_) {
        auto t = c->GetTalismanType();
        if (t != Card::TalismanType::None) {
            talismans_.insert(t);
        }
    }
}

int Player::GetScore() const
{
    return score_;
}

void Player::OrganizeHand(Card* c, const Card* insertBefore)
{
    if (c == insertBefore) {
        return;
    }
    if (!HasCard(c) || !HasCard(insertBefore)) {
        return;
    }
    hand_.remove(c);
    auto pos = std::find(hand_.begin(), hand_.end(), insertBefore);
    hand_.insert(pos, c);
}
} // namespace Engine
