#include <memory>
#include <optional>

#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>

#include <mc/world/actor/player/Player.h>

#include <LOICollectionA/base/Wrapper.h>
#include <LOICollectionA/base/ServiceProvider.h>

#include <LOICollectionA/include/Plugins/PvpPlugin.h>
#include <LOICollectionA/include/Plugins/MutePlugin.h>
#include <LOICollectionA/include/Plugins/ChatPlugin.h>
#include <LOICollectionA/include/Plugins/LanguagePlugin.h>

#include <gmlib/gm/papi/PlaceholderAPI.h>

#include "include/RegistryHelper.h"

#include "include/Events/APIEngineEvent.h"

#include "utils/SystemUtils.h"

#include "ConfigPlugin.h"

#include "include/Expands/GMLIBExpand.h"

namespace LOICollectionA::Expands {
    struct GMLIBExpand::Impl {
        C_Config::C_GMLIB options;

        ll::event::ListenerPtr APIUtilsTranslateStringEventListener;
    };

    GMLIBExpand::GMLIBExpand() : mImpl(std::make_unique<Impl>()) {}
    GMLIBExpand::~GMLIBExpand() = default;

    GMLIBExpand& GMLIBExpand::getInstance() {
        static GMLIBExpand instance;
        return instance;
    }

    void GMLIBExpand::registerVariable() {
        if (!this->mImpl->options.APIEngineVariable)
            return;

        gmlib::papi::PlaceholderAPI::getInstance().registerPlaceholder("lca_player_title", [](
            optional_ref<Actor> actor, ll::SmallStringMap<std::string> const&, std::string const&
        ) -> std::optional<std::string> {
            if (!actor.has_value() || !((Actor*)actor.as_ptr())->isPlayer())
                return std::nullopt;

            auto* player = (Player*)actor.as_ptr();
            return LOICollection::Plugins::ChatPlugin::getInstance().getTitle(*player);
        });
        gmlib::papi::PlaceholderAPI::getInstance().registerPlaceholder("lca_player_title_time", [](
            optional_ref<Actor> actor, ll::SmallStringMap<std::string> const&, std::string const&
        ) -> std::optional<std::string> {
            if (!actor.has_value() || !((Actor*)actor.as_ptr())->isPlayer())
                return std::nullopt;

            auto* player = (Player*)actor.as_ptr();
            return SystemUtils::toFormatTime(
                LOICollection::Plugins::ChatPlugin::getInstance().getTitleTime(*player, LOICollection::Plugins::ChatPlugin::getInstance().getTitle(*player)), "None"
            );
        });
        gmlib::papi::PlaceholderAPI::getInstance().registerPlaceholder("lca_player_pvp", [](
            optional_ref<Actor> actor, ll::SmallStringMap<std::string> const&, std::string const&
        ) -> std::optional<std::string> {
            if (!actor.has_value() || !((Actor*)actor.as_ptr())->isPlayer())
                return std::nullopt;

            auto* player = (Player*)actor.as_ptr();
            return LOICollection::Plugins::PvpPlugin::getInstance().isEnable(*player) ? "true" : "false";
        });
        gmlib::papi::PlaceholderAPI::getInstance().registerPlaceholder("lca_player_mute", [](
            optional_ref<Actor> actor, ll::SmallStringMap<std::string> const&, std::string const&
        ) -> std::optional<std::string> {
            if (!actor.has_value() || !((Actor*)actor.as_ptr())->isPlayer())
                return std::nullopt;

            auto* player = (Player*)actor.as_ptr();
            return LOICollection::Plugins::MutePlugin::getInstance().isMute(*player) ? "true" : "false";
        });
        gmlib::papi::PlaceholderAPI::getInstance().registerPlaceholder("lca_player_language", [](
            optional_ref<Actor> actor, ll::SmallStringMap<std::string> const&, std::string const&
        ) -> std::optional<std::string> {
            if (!actor.has_value() || !((Actor*)actor.as_ptr())->isPlayer())
                return std::nullopt;

            auto* player = (Player*)actor.as_ptr();
            return LOICollection::Plugins::LanguagePlugin::getInstance().getLanguage(*player);
        });
    }

    void GMLIBExpand::listenEvent() {
        ll::event::EventBus& eventBus = ll::event::EventBus::getInstance();
        this->mImpl->APIUtilsTranslateStringEventListener = eventBus.emplaceListener<LOICollectionA::Events::APIUtilsTranslateStringEvent>([this](LOICollectionA::Events::APIUtilsTranslateStringEvent& event) -> void {
            if (!this->mImpl->options.APIEngineVariable)
                return;

            std::string str = event.getContent();

            event.getContent() = static_cast<std::string>(gmlib::papi::PlaceholderAPI::getInstance().translate(str, event.getPlayer()));
        });
    }

    void GMLIBExpand::unlistenEvent() {
        ll::event::EventBus& eventBus = ll::event::EventBus::getInstance();
        eventBus.removeListener(this->mImpl->APIUtilsTranslateStringEventListener);
    }

    bool GMLIBExpand::load() {
        if (!ServiceProvider::getInstance().getService<ReadOnlyWrapper<C_Config>>("Config")->get().GMLIB.ModuleEnabled)
            return false;

        this->mImpl->options = ServiceProvider::getInstance().getService<ReadOnlyWrapper<C_Config>>("Config")->get().GMLIB;

        return true;
    }

    bool GMLIBExpand::unload() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        this->mImpl->options = {};

        return true;
    }

    bool GMLIBExpand::registry() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        this->listenEvent();
        this->registerVariable();

        return true;
    }

    bool GMLIBExpand::unregistry() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        this->unlistenEvent();

        return true;
    }
}

REGISTRY_HELPER("GMLIBExpand", LOICollectionA::Expands::GMLIBExpand, LOICollectionA::Expands::GMLIBExpand::getInstance())
