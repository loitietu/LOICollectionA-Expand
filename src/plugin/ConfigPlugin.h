#pragma once

#include <string>

struct C_Config {
    int version = 0;
    struct C_GMLIB {
        bool ModuleEnabled = false;
        bool APIEngineVariable = true;
    } GMLIB;
    struct C_ConfigEditor {
        bool ModuleEnabled = false;
        bool RegisteryCommand = true;
        bool MonitorConfigFile = true;
    } ConfigEditor;
};

namespace Config {
    std::string GetVersion();

    void SynchronousPluginConfigVersion(C_Config& config);
    void SynchronousPluginConfigType(C_Config& config, const std::string& path);
}