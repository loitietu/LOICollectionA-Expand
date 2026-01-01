#pragma once

#include <memory>
#include <string>

namespace ll::io {
    class Logger;
}

namespace LOICollectionA::Expands {
    class ConfigExpand {
    public:
        static ConfigExpand& getInstance();

        ll::io::Logger* getLogger();

        void reload();
        void reload(const std::string& name);
        
        bool isValid();
        
    public:
        bool load();
        bool unload();
        bool registry();
        bool unregistry();

    private:
        ConfigExpand();
        ~ConfigExpand();

        void registeryCommand();
        void listenEvent();
        void unlistenEvent();

        struct operation;

        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
