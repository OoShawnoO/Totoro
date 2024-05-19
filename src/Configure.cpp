#include "core/Configure.h"

namespace totoro {

    Configure::Configure(const std::string &filePath) {
        std::ifstream in(filePath);
        conf = Json::parse(in);
    }

    Configure Configure::config("etc/config.json");

} // totoro