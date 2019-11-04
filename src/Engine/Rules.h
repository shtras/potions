#pragma once

namespace Engine
{
struct Rules
{
    size_t MaxHandToDraw = 7;
    size_t InitialClosetSize = 4;
    size_t InitialHandSize = 4;
    size_t MinPlayers = 2;
    size_t MaxPlayers = 6;
    size_t MinCardsInHand = 5;
};
} // namespace Engine
