#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace LOICollectionA::modules {
    class ModRegistry;
    class ModManager {
    public:
        static ModManager& getInstance();

        ModManager(const ModManager&) = delete;
        ModManager& operator=(const ModManager&) = delete;

        ModManager(ModManager&&) = delete;
        ModManager& operator=(ModManager&&) = delete;

        void registry(std::unique_ptr<ModRegistry> registry);
        void unregistry(const std::string& name);

        [[nodiscard]] ModRegistry* getRegistry(const std::string& name) const;

        [[nodiscard]] std::vector<std::string> mods() const;

    private:
        ModManager() = default;
        ~ModManager() = default;

        std::unordered_map<std::string, std::unique_ptr<ModRegistry>> mRegistries;
    };
}
