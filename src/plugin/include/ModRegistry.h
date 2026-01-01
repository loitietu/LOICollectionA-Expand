#pragma once

#include <memory>
#include <string>
#include <functional>

namespace LOICollectionA::modules {
    class ModRegistry {
    public:
        using CallBackType = bool();
        using CallBackFunc = std::function<CallBackType>;

        explicit ModRegistry(const std::string& name);
        ~ModRegistry();

        [[nodiscard]] std::string getName() const noexcept;

        bool onLoad();
        bool onUnload();
        bool onRegistry();
        bool onUnregistry();

        void onLoad(CallBackFunc func);
        void onUnload(CallBackFunc func);
        void onRegistry(CallBackFunc func);
        void onUnregistry(CallBackFunc func);

        void release() noexcept;

        [[nodiscard]] bool isLoaded() const noexcept;
        [[nodiscard]] bool isUnloaded() const noexcept;
        [[nodiscard]] bool isRegistered() const noexcept;
        [[nodiscard]] bool isUnregistered() const noexcept;

    private:
        struct Impl;

        std::unique_ptr<Impl> mImpl;
    };
}
