#pragma once
#include <string>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "bsoncxx_wrap.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Move
{
public:
    enum class Action { Draw, Skip, Assemble, Discard, Cast, EndTurn };
    struct Part
    {
        int id;
        Requirement::Type type;
    };
    Move(std::string& user);
    bool Parse(std::string moveJson);
    bool FromJson(const bsoncxx::document::view& bson);
    void ToJson(bsoncxx::builder::stream::document& d) const;
    Action GetAction() const;
    const std::string& GetUser() const;
    int GetCard() const;
    std::vector<Card*> GetParts(World* world) const;

private:
    Action action_ = Action::Skip;
    int card_ = -1;
    std::list<Part> parts_;
    std::string user_;
};
} // namespace Engine
