#include "core/Configure.h"

namespace totoro {
    Configure::Configure(const std::string &filePath) {
        std::ifstream in(filePath);
        conf = Json::parse(in);
    }

    Json &totoro::Configure::Get() {
        static Configure config("etc/config.json");
        return config.conf;
    }
} // totoro