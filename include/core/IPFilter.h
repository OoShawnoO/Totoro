#ifndef TOTORO_IPFILTER_H
#define TOTORO_IPFILTER_H

#include <netinet/in.h>
#include <set>
#include "Json.hpp"

namespace totoro {
    using Json = nlohmann::json;

    class IPFilter {
        bool isAllowedAll               {true};
        std::set<in_addr_t> allowedIPs  ;
        std::set<in_addr_t> bannedIPs   ;
    public:
        explicit IPFilter(bool _isAllowedAll = true);
        bool AddBan(in_addr_t inAddr);
        bool AddAllow(in_addr_t inAddr);
        bool isBanned(in_addr_t inAddr);
        bool isAllowed(in_addr_t inAddr);
    };

} // totoro

#endif //TOTORO_IPFILTER_H
