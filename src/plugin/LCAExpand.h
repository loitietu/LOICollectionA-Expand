#pragma once

#include <ll/api/mod/NativeMod.h>

#include "ConfigPlugin.h"

namespace LOICollectionA {
    class Expand {
    public:
        static Expand& getInstance();

        Expand() : mSelf(*ll::mod::NativeMod::current()) {}

        [[nodiscard]] ll::mod::NativeMod& getSelf() const {
            return mSelf;
        }

        bool load();
        bool unload();
        bool enable();
        bool disable();

    private:
        ll::mod::NativeMod& mSelf;

        C_Config config;
    };
}
