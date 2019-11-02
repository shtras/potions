#pragma once

#include <set>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Player
{
public:
    Player(World* w, std::string& user);

    void AddCard(Card* card);
    size_t HandSize() const;
    bool DiscardCard(Card* card);
    bool HasCard(Card* card);
    void AddAssembled(Card* card);
    void RemoveAssembled(Card* card);
    void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w, bool hidden = false) const;
    const std::string& GetUser() const;

private:
    bool removeFromHand(Card* card);

    World* world_ = nullptr;
    int score_ = 0;
    std::set<Card*> hand_;
    std::set<Card*> assembledCards_;
    std::string user_;
};
} // namespace Engine
