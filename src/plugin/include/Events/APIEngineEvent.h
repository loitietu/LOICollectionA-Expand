#pragma once

#include <string>

#include <ll/api/event/Event.h>

class Player;

namespace LOICollectionA::Events {
    class APIUtilsTranslateStringEvent final : public ll::event::Event {
    protected:
        std::string& mContent;

        Player& mPlayer;
    
    public:
        constexpr explicit APIUtilsTranslateStringEvent(
            Player& player,
            std::string& content
        ) : mContent(content), mPlayer(player) {}

        [[nodiscard]] std::string& getContent() const;
        [[nodiscard]] Player& getPlayer() const;
    };
}