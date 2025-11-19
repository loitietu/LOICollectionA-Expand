#pragma once

#include <string>

struct C_Config {
    int version = 0;
    struct C_GMLIB {
        bool ModuleEnabled = false;
        bool APIEngineVariable = true;
    } GMLIB;
};

namespace Config {
    std::string GetVersion();

    void SynchronousPluginConfigVersion(C_Config& config);
    void SynchronousPluginConfigType(C_Config& config, const std::string& path);
}