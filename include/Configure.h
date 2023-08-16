#ifndef TOTOROSERVER_CONFIGURE_H
#define TOTOROSERVER_CONFIGURE_H

#include <fstream>      /* ifstream */

#include "Json.hpp"     /* nlohmann::json */

namespace totoro {
    using json = nlohmann::json;
    class Configure {
        json conf;
        explicit Configure(const std::string& filePath);
    public:
        /**
         * @brief 获取配置文件
         * @return 解析为json格式配置
         */
        static json& Get();
    };

} // totoro

#endif //TOTOROSERVER_CONFIGURE_H
