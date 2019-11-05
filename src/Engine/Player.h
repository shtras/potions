#pragma once

#include <set>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4265)
#endif
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
    void DiscardCard(Card* card);
    bool HasCard(Card* card) const;
    bool HasAssembled(Card* card) const;
    void AddAssembled(Card* card);
    void RemoveAssembled(Card* card);
    Card* FindAssembledWithPart(Card* part) const;
    void ToJson(bsoncxx::builder::stream::document& d, bool hidden = false) const;
    bool FromJson(const bsoncxx::document::view& bson);
    const std::string& GetUser() const;
    void AddScore(int score);

private:
    bool removeFromHand(Card* card);

    World* world_ = nullptr;
    int score_ = 0;
    std::set<Card*> hand_;
    std::set<Card*> assembledCards_;
    std::string user_;
};
} // namespace Engine
