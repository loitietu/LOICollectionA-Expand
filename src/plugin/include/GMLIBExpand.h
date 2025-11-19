#pragma once

#include <memory>

namespace LOICollectionA::Expands {
    class GMLIBExpand {
    public:
        static GMLIBExpand& getInstance() {
            static GMLIBExpand instance;
            return instance;
        }

        void registerVariable();
        
    public:
        bool load();
        bool unload();
        bool registry();
        bool unregistry();

    private:
        GMLIBExpand();
        ~GMLIBExpand();

        void listenEvent();
        void unlistenEvent();

        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
