#include "Configure.h"

namespace totoro {
    Configure::Configure(const std::string &filePath) {
        std::ifstream in(filePath);
        conf = json::parse(in);
    }

    json &totoro::Configure::Get() {
        static Configure config("etc/config.json");
        return config.conf;
    }
} // totoro