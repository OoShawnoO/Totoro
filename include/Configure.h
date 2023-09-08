#ifndef TOTOROSERVER_CONFIGURE_H
#define TOTOROSERVER_CONFIGURE_H

#include <fstream>      /* ifstream */

#include "Json.hpp"     /* nlohmann::json */

namespace totoro {
    using Json = nlohmann::json;
    class Configure {
        Json conf;
        explicit Configure(const std::string& filePath);
    public:
        /**
         * @brief 获取配置文件
         * @return 解析为json格式配置
         */
        static Json& Get();
    };

} // totoro

#endif //TOTOROSERVER_CONFIGURE_H
