#include <chrono>
#include <string>
#include <sstream>

#include "utils/SystemUtils.h"

namespace SystemUtils {
    std::string toFormatTime(const std::string& str, const std::string& defaultValue) {
        std::chrono::local_seconds mTp;
        if ((std::istringstream(str) >> std::chrono::parse("%Y%m%d%H%M%S", mTp)).fail())
            return defaultValue;
        
        return std::format("{:%Y-%m-%d %H:%M:%S}", mTp);
    }
}