#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <Windows.h>

#include <ll/api/io/Logger.h>
#include <ll/api/io/LoggerRegistry.h>

#include <ll/api/coro/CoroTask.h>
#include <ll/api/coro/InterruptableSleep.h>
#include <ll/api/thread/ThreadPoolExecutor.h>

#include <ll/api/mod/Manifest.h>
#include <ll/api/mod/ModManagerRegistry.h>

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/command/EnumName.h>

#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/server/commands/CommandOutputMessageType.h>

#include <LOICollectionA/base/Wrapper.h>
#include <LOICollectionA/base/Throttle.h>
#include <LOICollectionA/base/ServiceProvider.h>

#include <LOICollectionA/include/ModManager.h>
#include <LOICollectionA/include/ModRegistry.h>

#include "include/RegistryHelper.h"

#include "ConfigPlugin.h"

#include "include/Expands/ConfigExpand.h"

std::wstring StringToWString(const std::string& str, UINT codePage = CP_UTF8) {
    if (str.empty()) 
        return {};
    
    int size = MultiByteToWideChar(codePage, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size, 0);
    MultiByteToWideChar(codePage, 0, str.c_str(), (int)str.size(), &wstrTo[0], size);
    return wstrTo;
}

namespace LOICollectionA::Expands {
    enum class ConfigObjectModule;

    constexpr inline auto ConfigObjectModuleName = ll::command::enum_name_v<ConfigObjectModule>;

    struct ConfigExpand::operation {
        ll::command::SoftEnum<ConfigObjectModule> Modules;
    };

    struct ConfigExpand::Impl {
        Throttle mThrottle;

        C_Config::C_ConfigEditor options;

        std::shared_ptr<ll::io::Logger> logger;

        ll::thread::ThreadPoolExecutor mExecutor{ "ConfigExpand", std::max(static_cast<size_t>(std::thread::hardware_concurrency()) - 2, static_cast<size_t>(2)) };

        ll::coro::InterruptableSleep FileWatchTaskSleep;

        std::atomic<bool> FileWatchTaskRunning{ true };

        Impl() : mThrottle(std::chrono::seconds(1)) {}
    };

    ConfigExpand::ConfigExpand() : mImpl(std::make_unique<Impl>()) {}
    ConfigExpand::~ConfigExpand() = default;

    ConfigExpand& ConfigExpand::getInstance() {
        static ConfigExpand instance;
        return instance;
    }

    ll::io::Logger* ConfigExpand::getLogger() {
        return this->mImpl->logger.get();
    }

    void ConfigExpand::registeryCommand() {
        ll::command::CommandRegistrar::getInstance().tryRegisterSoftEnum(ConfigObjectModuleName, LOICollection::modules::ModManager::getInstance().mods());

        ll::command::CommandHandle& command = ll::command::CommandRegistrar::getInstance()
            .getOrCreateCommand("config", "Config Manager", CommandPermissionLevel::GameDirectors, CommandFlagValue::NotCheat | CommandFlagValue::Async);
        command.overload().text("reload").text("all").execute([this](CommandOrigin const&, CommandOutput&) -> void {
            this->reload();
        });
        command.overload<operation>().text("reload").text("module").text("Modules").execute(
            [this](CommandOrigin const&, CommandOutput&, operation const& param) -> void {
            this->reload(param.Modules);
        });
    };

    void ConfigExpand::listenEvent() {
        this->mImpl->FileWatchTaskRunning.store(true, std::memory_order_release);

        ll::coro::keepThis([this]() -> ll::coro::CoroTask<> {
            auto mod = ll::mod::ModManagerRegistry::getInstance().getMod("LOICollectionA");

            if (!mod) {
                this->getLogger()->error("Failed to get LOICollectionA mod.");
                co_return;
            }

            std::string directory = mod->getConfigDir().string();

            HANDLE hDir = CreateFileW(
                StringToWString(directory).c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                nullptr
            );

            if (hDir == INVALID_HANDLE_VALUE) {
                this->getLogger()->error("Failed to open directory handle for {}", directory);
                co_return;
            }

            BYTE buffer[1024];
            DWORD bytesReturned;

            while (this->mImpl->FileWatchTaskRunning.load(std::memory_order_acquire)) {
                co_await this->mImpl->FileWatchTaskSleep.sleepFor(std::chrono::seconds(1));

                if (!this->mImpl->FileWatchTaskRunning.load(std::memory_order_acquire))
                    break;

                if (ReadDirectoryChangesW(
                    hDir,
                    buffer,
                    sizeof(buffer),
                    TRUE,
                    FILE_NOTIFY_CHANGE_LAST_WRITE | 
                    FILE_NOTIFY_CHANGE_FILE_NAME |
                    FILE_NOTIFY_CHANGE_SIZE,
                    &bytesReturned,
                    nullptr,
                    nullptr)) {

                    auto* info = 
                        reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                    
                    do {
                        std::wstring filename(info->FileName, info->FileNameLength / sizeof(WCHAR));
                        
                        switch (info->Action) {
                            case FILE_ACTION_MODIFIED:
                                if (filename.find(L"config.json") == std::wstring::npos)
                                    break;

                                this->mImpl->mThrottle([this]() -> void {
                                    this->reload();
                                });

                                break;
                            default:
                                break;
                        }
                        
                        info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
                    } while (info->NextEntryOffset != 0);
                }
            }

            CloseHandle(hDir);
        }).launch(this->mImpl->mExecutor);
    }
    
    void ConfigExpand::unlistenEvent() {
        this->mImpl->FileWatchTaskRunning.store(false, std::memory_order_release);

        this->mImpl->FileWatchTaskSleep.interrupt();
    }

    void ConfigExpand::reload() {
        if (!this->isValid())
            return;

        this->getLogger()->info("Reloading all config files.");

        std::vector<std::string> mMods = LOICollection::modules::ModManager::getInstance().mods();
        for (auto& mod : mMods) {
            LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(mod);

            if (!mRegistry) {
                this->getLogger()->warn("Mod {} not found.", mod);
                continue;
            }

            mRegistry->onUnload();
            mRegistry->onLoad();
        }

        this->getLogger()->info("All config files reloaded.");
    }
    
    void ConfigExpand::reload(const std::string& name) {
        if (!this->isValid())
            return;

        this->getLogger()->info("Reloading config file of module {}.", name);

        LOICollection::modules::ModRegistry* mRegistry = LOICollection::modules::ModManager::getInstance().getRegistry(name);

        if (!mRegistry) {
            this->getLogger()->warn("Mod {} not found.", name);
            return;
        }

        mRegistry->onUnload();
        mRegistry->onLoad();

        this->getLogger()->info("Config file of module {} reloaded.", name);
    }

    bool ConfigExpand::isValid() {
        return this->getLogger() != nullptr;
    }

    bool ConfigExpand::load() {
        if (!ServiceProvider::getInstance().getService<ReadOnlyWrapper<C_Config>>("Config")->get().ConfigEditor.ModuleEnabled)
            return false;

        this->mImpl->logger = ll::io::LoggerRegistry::getInstance().getOrCreate("LOICollectionA-Expand");
        this->mImpl->options = ServiceProvider::getInstance().getService<ReadOnlyWrapper<C_Config>>("Config")->get().ConfigEditor;

        return true;
    }

    bool ConfigExpand::unload() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        this->mImpl->logger.reset();
        this->mImpl->options = {};

        return true;
    }

    bool ConfigExpand::registry() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        if (this->mImpl->options.RegisteryCommand)
            this->registeryCommand();

        if (this->mImpl->options.MonitorConfigFile)
            this->listenEvent();

        return true;
    }

    bool ConfigExpand::unregistry() {
        if (!this->mImpl->options.ModuleEnabled)
            return false;

        if (this->mImpl->options.MonitorConfigFile)
            this->unlistenEvent();

        return true;
    }
}

REGISTRY_HELPER("ConfigExpand", LOICollectionA::Expands::ConfigExpand, LOICollectionA::Expands::ConfigExpand::getInstance())
