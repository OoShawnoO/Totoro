#ifndef TOTORO_HTTPSBASE_H
#define TOTORO_HTTPSBASE_H

#include "HttpBase.h"           /* HttpBase */
#include "utils/SSLSocket.h"     /* SSLSocket */

namespace totoro {
    /**
     * @brief @brief 负责HTTPS连接相关事务 / Response HTTPS connection transactions
     */
    class HttpsBase : public virtual HttpBase, public virtual SSLSocket {
    public:
        int Init(const ConnectionInitParameter &connectionInitParameter) override;

    protected:
        void Handler();

    public:
        int Close() override;
    };

} // totoro

#endif //TOTORO_HTTPSBASE_H
