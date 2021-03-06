#pragma once
#include <string>

#include "bsoncxx_wrap.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Move
{
public:
    enum class Action {
        Draw,
        Skip,
        Assemble,
        Disassemble,
        Discard,
        Cast,
        EndTurn,
        OrganizeHand,
        Unknown
    };
    struct Part
    {
        int id;
        Requirement::Type type;
        std::string player;
    };
    explicit Move(std::string user);
    bool Parse(std::string moveJson);
    bool FromJson(const bsoncxx::document::view& bson);
    void ToJson(bsoncxx::builder::stream::document& d) const;
    Action GetAction() const;
    const std::string& GetUser() const;
    const std::string& GetDeckType() const;
    const std::string& GetPlayer() const;
    int GetCard() const;
    int GetIngredient() const;
    bool IsService() const;
    std::vector<Card*> GetParts(World* world) const;

private:
    Action action_ = Action::Skip;
    int card_ = -1;
    int ingredient_ = -1;
    std::list<Part> parts_;
    std::string user_;
    std::string deckType_;
    std::string player_;
    Action actionFromString(std::string_view str);
    const std::map<Action, std::string_view> actionNames_ = {{Action::Draw, "draw"},
        {Action::Skip, "skip"}, {Action::Disassemble, "disassemble"},
        {Action::Assemble, "assemble"}, {Action::Discard, "discard"}, {Action::Cast, "cast"},
        {Action::OrganizeHand, "organizehand"}, {Action::EndTurn, "endturn"}};
};
} // namespace Engine
