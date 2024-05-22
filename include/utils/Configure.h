#ifndef TOTORO_CONFIGURE_H
#define TOTORO_CONFIGURE_H

#include <fstream>      /* ifstream */

#include "utils/Json.hpp"     /* nlohmann::json */

namespace totoro {
    using Json = nlohmann::json;

    /**
     * @brief 配置文件解析与数据获取 \n Configure file parse and get data
     */
    class Configure {
        explicit Configure(const std::string &filePath);

    public:
        Json conf;

        static Configure config;
    };

} // totoro

#endif //TOTORO_CONFIGURE_H
