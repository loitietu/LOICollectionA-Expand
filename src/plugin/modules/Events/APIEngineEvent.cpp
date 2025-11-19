#include <memory>
#include <string>

#include <ll/api/memory/Hook.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EmitterBase.h>
#include <ll/api/event/EventBus.h>

#include <mc/world/actor/player/Player.h>

#include <LOICollectionA/include/APIUtils.h>

#include "include/Events/APIEngineEvent.h"

namespace LOICollectionA::Events {
    std::string& APIUtilsTranslateStringEvent::getContent() const {
        return mContent;
    }

    Player& APIUtilsTranslateStringEvent::getPlayer() const {
        return mPlayer;
    }

    LL_STATIC_HOOK(
        APIUtilsTranslateStringHook,
        HookPriority::Normal,
        LOICollection::LOICollectionAPI::translateString,
        std::string,
        const std::string& str,
        Player& player
    ) {
        std::string result = origin(str, player);

        APIUtilsTranslateStringEvent event(player, result);
        ll::event::EventBus::getInstance().publish(event);

        return result;
    }

    LL_STATIC_HOOK(
        APIUtilsGetVariableStringHook,
        HookPriority::Normal,
        LOICollection::LOICollectionAPI::getVariableString,
        std::string,
        const std::string& str,
        Player& player
    ) {
        std::string result = origin(str, player);

        APIUtilsTranslateStringEvent event(player, result);
        ll::event::EventBus::getInstance().publish(event);

        return result;
    }

    static std::unique_ptr<ll::event::EmitterBase> emitterFactory();
    class APIUtilsTranslateStringEventEmitter : public ll::event::Emitter<emitterFactory, APIUtilsTranslateStringEvent> {
        ll::memory::HookRegistrar<APIUtilsTranslateStringHook> hook;
    };

    static std::unique_ptr<ll::event::EmitterBase> emitterFactory() {
        return std::make_unique<APIUtilsTranslateStringEventEmitter>();
    }
}
