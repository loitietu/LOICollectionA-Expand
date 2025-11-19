#include <filesystem>

#include <ll/api/Config.h>
#include <ll/api/io/Logger.h>
#include <ll/api/Mod/NativeMod.h>
#include <ll/api/Mod/RegisterHelper.h>

#include <LOICollectionA/include/ModManager.h>
#include <LOICollectionA/include/ModRegistry.h>

#include <LOICollectionA/base/Wrapper.h>
#include <LOICollectionA/base/ServiceProvider.h>

#include "ConfigPlugin.h"

#include "LCAExpand.h"

namespace LOICollectionA {
    Expand& Expand::getInstance() {
        static Expand instance;
        return instance;
    }

    bool Expand::load() {
        ll::io::Logger& logger = this->mSelf.getLogger();

        const std::filesystem::path& configDataPath = this->mSelf.getConfigDir();
        const std::filesystem::path& configFilePath = configDataPath / "config.json";

        Config::SynchronousPluginConfigVersion(this->config);

        logger.info("Loading LOICollectionA - Expand (Version {})", Config::GetVersion());
        logger.info("Protocol - Mojang Eula (https://account.mojang.com/documents/minecraft_eula)");

        if (!std::filesystem::exists(configFilePath)) {
            logger.info("Plugin - Configurations not found.");
            logger.info("Plugin - Saving default configurations.");

            if (!ll::config::saveConfig(this->config, configFilePath)) {
                logger.error("Failed to save default configurations.");
                
                return false;
            }
        }

        Config::SynchronousPluginConfigType(this->config, configFilePath.string());
        if (!ll::config::loadConfig(this->config, configFilePath)) {
            logger.info("Plugin - Update configurations.");

            if (!ll::config::saveConfig(this->config, configFilePath)) {
                logger.error("Failed to save default configurations.");

                return false;
            }
        }

        ServiceProvider::getInstance().registerInstance<ReadOnlyWrapper<C_Config>>(
            std::make_shared<ReadOnlyWrapper<C_Config>>(this->config), "Config"
        );

        logger.info("Initialization of configurations completed.");

        std::vector<std::string> mMods = LOICollection::modules::ModManager::getInstance().mods();
        std::for_each(mMods.begin(), mMods.end(), [&logger](const std::string& mod) -> void {
            LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(mod);

            if (!mRegistry) {
                logger.error("Failed to get mod registry for mod {}", mod);
                return;
            }

            mRegistry->onLoad();
        });

        logger.info("Initialization of plugins completed.");

        return true;
    }

    bool Expand::unload() {
        ll::io::Logger& logger = this->mSelf.getLogger();

        std::vector<std::string> mMods = LOICollection::modules::ModManager::getInstance().mods();
        std::for_each(mMods.begin(), mMods.end(), [&logger](const std::string& mod) -> void {
            LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(mod);

            if (!mRegistry) {
                logger.error("Failed to get mod registry for mod {}", mod);
                return;
            }

            mRegistry->onUnload();
        });

        return true;
    }

    bool Expand::enable() {
        ll::io::Logger& logger = this->mSelf.getLogger();

        std::vector<std::string> mMods = LOICollection::modules::ModManager::getInstance().mods();
        std::for_each(mMods.begin(), mMods.end(), [&logger](const std::string& mod) -> void {
            LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(mod);

            if (!mRegistry) {
                logger.error("Failed to get mod registry for mod {}", mod);
                return;
            }

            mRegistry->onRegistry();
        });

        return true;
    }

    bool Expand::disable() {
        ll::io::Logger& logger = this->mSelf.getLogger();

        std::vector<std::string> mMods = LOICollection::modules::ModManager::getInstance().mods();
        std::for_each(mMods.begin(), mMods.end(), [&logger](const std::string& mod) -> void {
            LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(mod);

            if (!mRegistry) {
                logger.error("Failed to get mod registry for mod {}", mod);
                return;
            }

            mRegistry->onUnregistry();
        });

        return true;
    }
}

LL_REGISTER_MOD(LOICollectionA::Expand, LOICollectionA::Expand::getInstance());
