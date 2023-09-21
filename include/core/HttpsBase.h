#ifndef TOTORO_HTTPSBASE_H
#define TOTORO_HTTPSBASE_H

#include "HttpBase.h"           /* HttpBase */
#include "core/SSLSocket.h"     /* SSLSocket */

namespace totoro {
    /**
     * @brief @brief 负责HTTPS连接相关事务 / Response HTTPS connection transactions
     */
    class HttpsBase : public virtual HttpBase,public SSLSocket{
    public:
        int Init(const ConnectionInitParameter &connectionInitParameter) override;

    protected:
        CallbackReturnType ReadCallback() override;
        CallbackReturnType AfterReadCallback() override;
        CallbackReturnType WriteCallback() override;
        CallbackReturnType AfterWriteCallback() override;

    public:
        int Close() override;
    };

} // totoro

#endif //TOTORO_HTTPSBASE_H
