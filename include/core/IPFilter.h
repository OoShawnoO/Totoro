#ifndef TOTORO_IPFILTER_H
#define TOTORO_IPFILTER_H

#include <netinet/in.h>
#include <set>
#include "Json.hpp"

namespace totoro {
    using Json = nlohmann::json;
    /**
     * @brief IP拦截与过滤 \n IP intercept and filter
     */
    class IPFilter {
        bool isAllowedAll               {true};
        std::set<in_addr_t> allowedIPs  ;
        std::set<in_addr_t> bannedIPs   ;
    public:
        explicit IPFilter(bool _isAllowedAll = true);
        // 添加拦截IP / Add intercept IP
        bool AddBan(in_addr_t inAddr);
        // 添加允许的IP / Add allowed IP
        bool AddAllow(in_addr_t inAddr);
        // 判断是否要拦截 / Judge whether intercept or not
        bool isBanned(in_addr_t inAddr);
        // 判断是否允许 / Judge whether allow or not
        bool isAllowed(in_addr_t inAddr);
    };

} // totoro

#endif //TOTORO_IPFILTER_H
